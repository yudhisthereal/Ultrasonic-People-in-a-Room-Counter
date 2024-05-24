#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DEBUG_MODE

#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define ROOM_CAPACITY 3 // Less capacity means faster checks (closer upper and lower boundaries)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define ROOM_CAPACITY 20
#endif

#define DETECT_DISTANCE 30
#define FLAG_TIMEOUT 7000
#define LCD_DELAY 1000

#define RST_BTN_PIN 6
#define ECHO_S1_PIN 3
#define TRIG_S1_PIN 4
#define ECHO_S2_PIN 10
#define TRIG_S2_PIN 9
#define RELAY_PIN 12

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long distance1 = 0, distance2 = 0;
unsigned long lastFlagSet[2] = {0, 0};
unsigned long lastLcdUpdate = 0;
bool flag[2] = {false, false};
bool ledOn = false;
int peopleCount = 0;

/**
 * @brief Read distance from ultrasonic sensor
 *
 * @param triggerPin Trigger pin of the ultrasonic sensor
 * @param echoPin Echo pin of the ultrasonic sensor
 * @param distance Reference to store the distance read from the sensor
 */
void readUltrasonicSensor(int triggerPin, int echoPin, unsigned long &distance)
{
  long duration;

  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration / 29 / 2; // Convert time to distance in cm
}

/**
 * @brief Display people count on the LCD
 */
void displayCount()
{
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(4, 1);
  lcd.print("Count: ");
  lcd.print(peopleCount);
  lcd.setCursor(0, 0);
}

/**
 * @brief Display a message centered on the LCD
 *
 * @param msg Message to display
 * @param row Row number (0 or 1)
 */
void displayCenteredMessage(const char *msg, bool row = 0)
{
  short paddingLength = (16 - strlen(msg)) / 2;
  char padding[paddingLength + 1];
  memset(padding, ' ', paddingLength);
  padding[paddingLength] = '\0';

  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
  lcd.print(padding);
  lcd.print(msg);
  lcd.setCursor(0, 0);
}

/**
 * @brief Reset all sensor flags
 */
void resetFlags()
{
  flag[0] = flag[1] = false;
}

/**
 * @brief Check if a person entered the room
 */
void checkPersonIn()
{
  if (distance2 <= DETECT_DISTANCE && !flag[1])
  {
    flag[1] = true;
    lastFlagSet[1] = millis();

    if (flag[0])
    {
      if (peopleCount < ROOM_CAPACITY)
      {
        peopleCount++;
        displayCount();
        displayCenteredMessage("Person entered!");
      }
      else
      {
        lcd.clear();
        displayCenteredMessage("Room Full!");
      }
      delay(2000);
      updateSensorReadings();
      resetFlags();
    }
  }
}

/**
 * @brief Check if a person exited the room
 */
void checkPersonOut()
{

  if (distance1 <= DETECT_DISTANCE && !flag[0])
  {
    flag[0] = true;
    lastFlagSet[0] = millis();

    if (flag[1])
    {
      if (peopleCount > 0)
      {
        peopleCount--;
        displayCount();
        displayCenteredMessage("Person exited!");
      }
      else
      {
        lcd.clear();
        displayCenteredMessage("Ghost exited!?");
      }
      delay(2000);
      updateSensorReadings();
      resetFlags();
    }
  }
}

/**
 * @brief Display debug information on the LCD
 */
void displayDebugInfo()
{
  lcd.clear();

  lcd.print("S1: ");
  lcd.print(distance1);
  lcd.print("cm");
  lcd.setCursor(12, 0);
  lcd.print(" (");
  lcd.print(flag[0]);
  lcd.print(")");

  lcd.setCursor(0, 1);
  lcd.print("S2: ");
  lcd.print(distance2);
  lcd.print("cm");
  lcd.setCursor(12, 1);
  lcd.print(" (");
  lcd.print(flag[1]);
  lcd.print(")");
}

/**
 * @brief Display final information on the LCD
 */
void displayFinalInfo()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("People: ");
  lcd.print(peopleCount);
  lcd.setCursor(0, 1);
  lcd.print("Light is ");
  lcd.print(peopleCount > 0 ? "On" : "Off");
}

/**
 * @brief Update LED state based on people count
 */
void updateLedState()
{
  if (peopleCount > 0 && !ledOn)
  {
    digitalWrite(RELAY_PIN, HIGH);
    ledOn = true;
  }
  else if (peopleCount == 0 && ledOn)
  {
    digitalWrite(RELAY_PIN, LOW);
    ledOn = false;
  }
}

/**
 * @brief Check if a flag is outdated, then reset it
 */
void checkFlagTimeout()
{
  for (int i = 0; i < 2; i++)
  {
    if (flag[i] && millis() - lastFlagSet[i] > FLAG_TIMEOUT)
    {
      flag[i] = false;
    }
  }
}

/**
 * @brief Handle reset button press
 * If the reset button is pressed, reset only the LCD
 */
void handleResetButton()
{
  if (digitalRead(RST_BTN_PIN))
  {
    delay(25);
    if (digitalRead(RST_BTN_PIN))
    {
      lcd.begin(16, 2);
      lcd.backlight();
      delay(1000);
    }
  }
}

/**
 * @brief Update sensor readings
 */
void updateSensorReadings()
{
  readUltrasonicSensor(TRIG_S1_PIN, ECHO_S1_PIN, distance1);
  delay(3);
  readUltrasonicSensor(TRIG_S2_PIN, ECHO_S2_PIN, distance2);
  delay(3);
}

/**
 * @brief Initialize serial communication
 *
 * @param baudRate Baud rate of the serial communication
 */
void initializeSerial(unsigned long baudRate)
{
  Serial.begin(baudRate);
  while (!Serial)
    ;
}

/**
 * @brief Initialize pin modes
 */
void initializePins()
{
  pinMode(RST_BTN_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(ECHO_S1_PIN, INPUT);
  pinMode(ECHO_S2_PIN, INPUT);
  pinMode(TRIG_S1_PIN, OUTPUT);
  pinMode(TRIG_S2_PIN, OUTPUT);
}

/**
 * @brief Initialize the LCD
 */
void initializeLcd()
{
  lcd.init();
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
}

/**
 * @brief Greet the user upon startup
 */
void greetUser()
{
  lcd.print("     Welcome    ");
  DEBUG_PRINTLN("System Ready");
  lastLcdUpdate = millis();
}

/**
 * @brief Update the display on the LCD
 */
void updateDisplay()
{
  if (millis() - lastLcdUpdate >= LCD_DELAY)
  {
    lastLcdUpdate = millis();
#ifdef DEBUG_MODE
    displayDebugInfo();
#else
    displayFinalInfo();
#endif
  }
}

void setup()
{
  initializeSerial(115200);
  initializePins();
  initializeLcd();
  greetUser();
  resetFlags();
}

void loop()
{
  handleResetButton();

  updateSensorReadings();
  checkPersonIn();
  checkPersonOut();
  checkFlagTimeout();

  updateDisplay();
  updateLedState();
}
