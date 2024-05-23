# 1 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino"
# 2 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino" 2
# 3 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino" 2
// #include <Arduino_FreeRTOS.h>
# 27 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino"
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long dis_a = 0, dis_b = 0;
unsigned long last_flag_set[2] = {0, 0};
bool flag[2] = {false, false};
bool led_on = false;
int people_count = 0;

/**
 * @brief Read ultrasonic sensor distance
 * 
 * @param pin_t Trigger pin
 * @param pin_e Echo pin
 * @param ultra_dist Variable to store distance
 */
void ultra_read(int pin_t, int pin_e, unsigned long &ultra_dist) {
  long time;

  digitalWrite(pin_t, 0x0);
  delayMicroseconds(2);

  digitalWrite(pin_t, 0x1);
  delayMicroseconds(10);
  digitalWrite(pin_t, 0x0);

  time = pulseIn(pin_e, 0x1);

  ultra_dist = time / 29 / 2;
}

/**
 * @brief Display person count on LCD
 */
void display_count() {
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
void display_center(char *msg, bool row = 0, short d = 1000) {
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
void reset_flags() {
  flag[0] = flag[1] = false;
}

/**
 * @brief Check if a person enters
 */
void check_person_in() {
  if (dis_b < 30 && !flag[1]) {
    flag[1] = true;
    last_flag_set[1] = millis();

    if (flag[0]) {
      reset_flags();
      if (people_count < 3 /* less capacity means faster checks (closer upper and lower boundaries)*/) {
        people_count++;
        display_count();
        display_center("Person entered!");
      } else {
        display_center("Room Full!");
      }
    }
  }
}

/**
 * @brief Check if a person exits
 */
void check_person_out() {
  if (dis_a < 30 && !flag[0]) {
    flag[0] = true;
    last_flag_set[0] = millis();

    if (flag[1]) {
      reset_flags();
      if (people_count > 0) {
        people_count--;
        display_count();
        display_center("Person exited!");
      } else {
        display_center("Ghost exited!?");
      }
    }
  }
}

/**
 * @brief Developer mode LCD display
 */
void dev_display() {
  lcd.clear();

  if (people_count == 3 /* less capacity means faster checks (closer upper and lower boundaries)*/) {
    lcd.print("Room Full");
    display_count();
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
void final_display() {
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
void update_led() {
  if (people_count > 0 && !led_on) {
    digitalWrite(12, 0x1);
    led_on = true;
  } else if (people_count == 0 && led_on) {
    digitalWrite(12, 0x0);
    led_on = false;
  }
}

/**
 * @brief Check if a flag is outdated, then resets it
 */
void check_flag_timeout() {
  for (int i = 0; i < 2; i++) {
    if (flag[i]) {
      if (millis() - last_flag_set[i] > 7000) {
        flag[i] = false;
      }
    }
  }
}

/**
 * @brief Handle reset button press
 * If the reset button is pressed, we reset ONLY THE LCD
 */
void handle_reset_btn() {
  if (!digitalRead(6)) {
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
void initLcd() {
  lcd.init();
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
}

/**
 * @brief Greet the user upon startup
 */
void greet() {
  lcd.print("     Welcome    ");
  Serial.println("System Ready");
}

void setup() {
  initSerial(115200);
  initPins();
  initLcd();
  greet();
}

void loop() {
  handle_reset_btn();

  ultra_read(4, 3, dis_a); delay(10);
  ultra_read(9, 10, dis_b); delay(10);
  check_person_in();
  check_person_out();
  check_flag_timeout();

  // final_display();
  update_led();
  dev_display();
  delay(100);
}
