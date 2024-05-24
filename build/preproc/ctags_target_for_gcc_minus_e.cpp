# 1 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino"
# 2 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino" 2
# 3 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino" 2
# 4 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino" 2

// #define DEBUG_MODE
# 33 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino"
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long distance1 = 0, distance2 = 0;
unsigned long lastFlagSet[2] = {0, 0};
unsigned long lastLcdUpdate = 0;
unsigned long lcdPausedAt = 0;
int lcdPauseDuration = 0;
bool flag[2] = {false, false};
bool ledOn = false;
byte lastCounterEvent = 0;
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

  digitalWrite(triggerPin, 0x0);
  delayMicroseconds(2);
  digitalWrite(triggerPin, 0x1);
  delayMicroseconds(10);
  digitalWrite(triggerPin, 0x0);

  duration = pulseIn(echoPin, 0x1);
  distance = duration / 29 / 2; // Convert time to distance in cm
}

/**
 * @brief pauses the lcd main update (non blocking)
 */
void pauseLcd(int duration)
{
  lcdPausedAt = millis();
  lcdPauseDuration = duration;
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
  lastCounterEvent = 0;
}

/**
 * @brief Check if a person entered the room
 */
void checkPersonIn()
{
  if (distance2 <= 30 && !flag[1])
  {
    flag[1] = true;
    lastFlagSet[1] = millis();

    if (flag[0])
    {
      if (peopleCount < 20)
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

      lastCounterEvent = 1;
      pauseLcd(1500);
    }
  }
}

/**
 * @brief Check if a person exited the room
 */
void checkPersonOut()
{

  if (distance1 <= 30 && !flag[0])
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

      lastCounterEvent = 2;
      pauseLcd(1500);
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
  lcd.print("/");
  lcd.print(20);
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
    digitalWrite(12, 0x1);
    ledOn = true;
  }
  else if (peopleCount == 0 && ledOn)
  {
    digitalWrite(12, 0x0);
    ledOn = false;
  }
}

/**
 * @brief Check if a flag is outdated, then reset it
 */
void attemptFlagReset()
{
  // if a person just entered/exited the room
  // but the last sensor detecting the person is still detecting the person
  // i.e. the person is still in the detecting range after being counted
  // we don't want to reset the flags, which would cause bugs.
  // only reset when the person gets out of range.
  if (lastCounterEvent != 0)
  {
    delay(100);
    if (distance1 > 30 && distance2 > 30)
    {
      resetFlags();
    }
    return;
  }

  // check for outdated flags, tries to reset them
  for (int i = 0; i < 2; i++)
  {
    if (flag[i] && millis() - lastFlagSet[i] > 3500)
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
  if (digitalRead(6))
  {
    delay(25);
    if (digitalRead(6))
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
  readUltrasonicSensor(4, 3, distance1);
  delay(3);
  readUltrasonicSensor(9, 10, distance2);
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
  pinMode(6, 0x2);
  pinMode(12, 0x1);
  pinMode(3, 0x0);
  pinMode(10, 0x0);
  pinMode(4, 0x1);
  pinMode(9, 0x1);
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
  ;
  lastLcdUpdate = millis();
}

/**
 * @brief Update the display on the LCD
 */
void updateDisplay(bool forced = false)
{
  if (millis() - lcdPausedAt < lcdPauseDuration && !forced)
  {
    return; // lcd paused
  }

  if (millis() - lastLcdUpdate >= 1000 || forced)
  {
    lastLcdUpdate = millis();



    displayFinalInfo();

  }
}

void savePeopleCount()
{
  EEPROM.update(1, peopleCount);
}

void loadPeopleCount()
{
  if (EEPROM.read(0) == true)
  {
    peopleCount = EEPROM.read(1);
  }
  else
  {
    peopleCount = 0;
    EEPROM.write(0, true);
  }
}

void setup()
{
  initializeSerial(115200);
  initializePins();
  initializeLcd();
  loadPeopleCount();
  resetFlags();
  greetUser();
  updateDisplay(true);
}

void loop()
{

  handleResetButton();

  updateSensorReadings();
  checkPersonIn();
  checkPersonOut();
  attemptFlagReset();
  savePeopleCount();

  updateDisplay();
  updateLedState();
}
