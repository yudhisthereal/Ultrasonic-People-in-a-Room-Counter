# 1 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino"
# 2 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino" 2
# 3 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino" 2
# 4 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino" 2

LiquidCrystal_I2C lcd(0x27, 16, 2); // Inisialisasi LCD I2C dengan alamat 0x27 dan ukuran 16x2
# 21 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/ultrasonic_person_counter.ino"
unsigned long dis_a = 0, dis_b = 0; // Variabel untuk menyimpan nilai dari sensor ultrasonic
unsigned long last_flag_set[2] = {0,0};
bool flag[2] = {false, false}; // Variabel flag untuk sensor ultrasonic
bool led_on = false; // apakah LED on?
int people_count = 0; // Variabel untuk menyimpan jumlah orang


// Fungsi untuk membaca nilai ultrasonic
void ultra_read(int pin_t, int pin_e, unsigned long &ultra_dist){
  long time;

  // pastikan pin_t (trigger) clear
  digitalWrite(pin_t, 0x0);
  delayMicroseconds(2);

  // trigger gelombang ultrasonik
  digitalWrite(pin_t, 0x1); // buat pin_t HIGH
  delayMicroseconds(10); // buat agar pin_t tetap HIGH selama 10us
  digitalWrite(pin_t, 0x0);

  // baca durasi HIGH di pin_e (echo)
  time = pulseIn(pin_e, 0x1);

  // dapetin jarak dari sensor ke objek yang dideteksi
  // pembagian dengan 29 karena biasanya kecepatan sinyal ultrasonik = 29us/cm 
  // pembagian dengan 2 karena waktu tempuh yang diterima sama dengan 2x waktu sensor ke objek-
  // karena sinyal ultrasoniknya bolak-balik (dari sensor ke objek, terus mantul dari objek ke sensor lagi)
  ultra_dist = time / 29 / 2;
}

// menampilkan jumlah orang dalam ruangan ("count: <jumlah_orang>")
void display_count() {
  lcd.setCursor(0,1);
  lcd.print("                "); // clear baris 2
  lcd.setCursor(4,1);
  lcd.print("count: ");
  lcd.print(people_count);
  lcd.setCursor(0,0);
  vPortDelay(1);
}

/**
 * @brief 
 * display a message centered in lcd in a certain row.
 * 
 * @param msg
 * message string to be displayed
 * 
 * @param row (optional)
 * 0 (default) means first row, 1 means second row
 * 
 * @param d (optional)
 * delay in ms after displaying msg (default 1000 ms)
*/
void display_center(char *msg, bool row = 0, short d = 1000) {
  short l = (16 - strlen(msg))/2; // panjang padding kanan kiri
  char padding[l];
  short i;
  for (i = 0; i < l; i++) {
    padding[i] = ' ';
  }
  padding[i] = '\0';

  lcd.setCursor(0,row);
  lcd.print("                "); // clear baris 1
  lcd.setCursor(0,row);
  lcd.print(padding);
  lcd.print(msg);
  lcd.setCursor(0,0);
  vPortDelay(d);
}

/**
 * @brief
 * resets all sensor flags
*/
void reset_flags() {
  flag[0] = flag[1] = false;
}

/**
 * @brief
 * check if a person enters.
 * checks if people count is maxed.
 * displays a message to the LCD on person entrance.
*/
void check_person_in() {

  if (dis_b < 30 /* jarak (cm) minimal agar pembacaan ultrasonik dianggap valid*/ && !flag[1]) {
    flag[1] = true;
    last_flag_set[1] = millis();

    if (flag[0]) {
      reset_flags();
      if (people_count < 5 /* jumlah maksimal orang dalam ruangan*/) {
        people_count++;
        display_count();
        display_center("Orang masuk!");
      } else {
        display_center("Ruang Penuh!");
      }
    }
  }
}

/**
 * @brief
 * check if a person exits.
 * checks if people count is 0.
 * displays a message to the LCD on person exit.
*/
void check_person_out() {

  if (dis_a < 30 /* jarak (cm) minimal agar pembacaan ultrasonik dianggap valid*/ && !flag[0]) {
    flag[0] = true;
    last_flag_set[0] = millis();

    if (flag[1]) {
      reset_flags();
      if (people_count > 0) {
        people_count--;
        display_count();
        display_center("Orang keluar!");
      } else {
        display_center("Hantu keluar!?");
      }
    }
  }
}

/**
 * @brief
 * Developer LCD display
*/
void dev_display() {
  lcd.clear();

  if (people_count == 5 /* jumlah maksimal orang dalam ruangan*/) {
    lcd.print("Ruangan Penuh");
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

  lcd.setCursor(0,1);
  lcd.print("S2: ");
  lcd.print(dis_b);
  lcd.print("cm");
  lcd.setCursor(12, 1);
  lcd.print(" (");
  lcd.print(flag[1]);
  lcd.print(")");
}

/**
 * @brief
 * Final (launch version) LCD display 
*/
void final_display() {
  lcd.setCursor(0, 0);
  lcd.print("People: ");
  lcd.print(people_count);
  lcd.print("  ");
  lcd.setCursor(0, 1);
  lcd.print("Light is ");
  if(people_count > 0){
    lcd.print("On");
  } else {
    lcd.print("Off");
  }
}

/**
 * @brief
 * updates LED state based on people_count
*/
void update_led() {
  if(people_count > 0 && !led_on) {
    digitalWrite(12 /* Relay pin terhubung ke pin 12 Arduino*/, 0x1);
    led_on = true;
  } else if (people_count == 0 && led_on) {
    digitalWrite(12 /* Relay pin terhubung ke pin 12 Arduino*/, 0x0);
    led_on = false;
  }
}

/**
 * @brief
 * checks if a flag is outdated, then resets it.
*/
void check_flag_timeout() {
  for (int i = 0; i < 2; i++) {
    if (flag[i]) {
      if (millis() - last_flag_set[i] > 7000 /* timeout reset flag1 dan 2 jika tidak ada orang masuk/keluar*/) {
        flag[i] = false;
      }
    }
  }
}

/**
 * @brief
 * if the reset button is pressed, we reset ONLY THE LCD
*/
void handle_reset_btn() {
  if (!digitalRead(6 /* LCD reset button*/)) {
    lcd.begin(16, 2);
    lcd.backlight();
  }
}

void setup(){
  Serial.begin(9600); // Inisialisasi komunikasi serial pada 9600 bps
  while (!Serial); // pastikan serial bener2 siap

  pinMode(6 /* LCD reset button*/, 0x2);
  pinMode(12 /* Relay pin terhubung ke pin 12 Arduino*/, 0x1); // Set pin relay sebagai output
  pinMode(3 /* Ultrasonic echo pin 1 terhubung ke pin 3 Arduino */, 0x0);
  pinMode(10 /* Ultrasonic echo pin 2 terhubung ke pin 10 Arduino */, 0x0);
  pinMode(4 /* Ultrasonic trigger pin 1 terhubung ke pin 4 Arduino */, 0x1);
  pinMode(9 /* Ultrasonic trigger pin 2 terhubung ke pin 9 Arduino */, 0x1);

  lcd.init();
  lcd.begin(16,2); // Inisialisasi LCD
  lcd.backlight(); // Nyalakan backlight LCD
  lcd.setCursor(0, 0);
  lcd.print("     Welcome    ");
  Serial.print("hello");
  vPortDelay(1000); // Tunggu sebentar
}

void loop(){
  handle_reset_btn();

  ultra_read(4 /* Ultrasonic trigger pin 1 terhubung ke pin 4 Arduino */, 3 /* Ultrasonic echo pin 1 terhubung ke pin 3 Arduino */, dis_a); vPortDelay(10); // Membaca nilai sensor ultrasonic 1
  ultra_read(9 /* Ultrasonic trigger pin 2 terhubung ke pin 9 Arduino */, 10 /* Ultrasonic echo pin 2 terhubung ke pin 10 Arduino */, dis_b); vPortDelay(10); // Membaca nilai sensor ultrasonic 2

  check_person_in();
  check_person_out();
  check_flag_timeout();

  // final_display();
  update_led();
  dev_display();
  vPortDelay(100); // Tambahkan delay untuk memperbarui tampilan LCD
}
