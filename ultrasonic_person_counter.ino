#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// #include <Arduino_FreeRTOS.h>
// #include <semphr.h>

#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define room_capacity 3 // less capacity means faster checks (closer upper and lower boundaries)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define room_capacity 20
#endif

#define detect_distance 30
#define flag_timeout 7000

#define reset_btn 6
#define e_s1 3
#define t_s1 4
#define e_s2 10
#define t_s2 9
#define relay 12


LiquidCrystal_I2C lcd(0x27, 16, 2);


unsigned long dis_a = 0, dis_b = 0;
unsigned long last_flag_set[2] = {0, 0};
bool flag[2] = {false, false};
bool led_on = false;
int people_count = 0;


void initSerial(unsigned long);
void initPins(void);
void initLcd(void);
void checkFlagTimeout(void);
void updateLed(void);

void setup() {
  initSerial(115200);
  initPins();
  initLcd();
}

void loop() {
  handleResetButton();

  ultraRead(t_s1, e_s1, dis_a); delay(10);
  ultraRead(t_s2, e_s2, dis_b); delay(10);
  checkPersonIn();
  checkPersonOut();
  checkFlagTimeout();

  // finalDisplay();
  updateLed();
  devDisplay();
  delay(100);
}

/**
 * @brief Read ultrasonic sensor distance
 * 
 * @param pin_t Trigger pin
 * @param pin_e Echo pin
 * @param ultra_dist Variable to store distance
 */
void ultraRead(int pin_t, int pin_e, unsigned long &ultra_dist) {
  long time;

  digitalWrite(pin_t, LOW);
  delayMicroseconds(2);

  digitalWrite(pin_t, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin_t, LOW);

  time = pulseIn(pin_e, HIGH);

  ultra_dist = time / 29 / 2;
}

/**
 * @brief Display person count on LCD
 */
void displayCount() {
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(4, 1);
  lcd.print("count: ");
  lcd.print(people_count);
  lcd.setCursor(0, 0);
  delay(1);
}

/**
 * @brief Display message centered on LCD
 * 
 * @param msg Message to display
 * @param row Row number (0 or 1)
 * @param d Delay after displaying the message
 */
void displayCenter(char *msg, bool row = 0, short d = 1000) {
  short l = (16 - strlen(msg))/2;
  char padding[l];
  short i;
  for (i = 0; i < l; i++) {
    padding[i] = ' ';
  }
  padding[i] = '\0';

  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
  lcd.print(padding);
  lcd.print(msg);
  lcd.setCursor(0, 0);
  delay(d);
}

/**
 * @brief Reset all sensor flags
 */
void resetFlags() {
  flag[0] = flag[1] = false;
}

/**
 * @brief Check if a person enters
 */
void checkPersonIn() {
  if (dis_b < detect_distance && !flag[1]) {
    flag[1] = true;
    last_flag_set[1] = millis();

    if (flag[0]) {
      resetFlags();
      if (people_count < room_capacity) {
        people_count++;
        displayCount();
        displayCenter("Person entered!");
      } else {
        displayCenter("Room Full!");
      }
    }
  }
}

/**
 * @brief Check if a person exits
 */
void checkPersonOut() {
  if (dis_a < detect_distance && !flag[0]) {
    flag[0] = true;
    last_flag_set[0] = millis();

    if (flag[1]) {
      resetFlags();
      if (people_count > 0) {
        people_count--;
        displayCount();
        displayCenter("Person exited!");
      } else {
        displayCenter("Ghost exited!?");
      }
    }
  }
}

/**
 * @brief Developer mode LCD display
 */
void devDisplay() {
  lcd.clear();

  if (people_count == room_capacity) {
    lcd.print("Room Full");
    displayCount();
    return;
  }

  lcd.print("S1: ");
  lcd.print(dis_a);
  lcd.print("cm");
  lcd.setCursor(12, 0);
  lcd.print(" (");
  lcd.print(flag[0]);
  lcd.print(")");

  lcd.setCursor(0, 1);
  lcd.print("S2: ");
  lcd.print(dis_b);
  lcd.print("cm");
  lcd.setCursor(12, 1);
  lcd.print(" (");
  lcd.print(flag[1]);
  lcd.print(")");
}

/**
 * @brief Final (launch version) LCD display
 */
void finalDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("People: ");
  lcd.print(people_count);
  lcd.print("  ");
  lcd.setCursor(0, 1);
  lcd.print("Light is ");
  if (people_count > 0) {
    lcd.print("On");
  } else {
    lcd.print("Off");
  }
}

/**
 * @brief Update LED state based on people_count
 */
void updateLed() {
  if (people_count > 0 && !led_on) {
    digitalWrite(relay, HIGH);
    led_on = true;
  } else if (people_count == 0 && led_on) {
    digitalWrite(relay, LOW);
    led_on = false;
  }
}

/**
 * @brief Check if a flag is outdated, then resets it
 */
void checkFlagTimeout() {
  for (int i = 0; i < 2; i++) {
    if (flag[i]) {
      if (millis() - last_flag_set[i] > flag_timeout) {
        flag[i] = false;
      }
    }
  }
}

/**
 * @brief Handle reset button press
 * If the reset button is pressed, we reset ONLY THE LCD
 */
void handleResetButton() {
  if (!digitalRead(reset_btn)) {
    lcd.begin(16, 2);
    lcd.backlight();
  }
}

/**
 * @brief Initialize Serial Monitor
 * 
 * @param baud Baudrate of the Serial Communication
 */
void initSerial(unsigned long baud) {
  Serial.begin(baud);
  while (!Serial);
}

/**
 * @brief Initialize pin modes
 */
void initPins() {
  pinMode(reset_btn, INPUT_PULLUP);
  pinMode(relay, OUTPUT);
  pinMode(e_s1, INPUT);
  pinMode(e_s2, INPUT);
  pinMode(t_s1, OUTPUT);
  pinMode(t_s2, OUTPUT);
}

/**
 * @brief Initialize the LCD
 */
void initLcd() {
  lcd.init();
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
}

/**
 * @brief Greets the user :)
 */
void greet() {
  lcd.print("     Welcome    ");
  DEBUG_PRINTLN("System Ready");
}