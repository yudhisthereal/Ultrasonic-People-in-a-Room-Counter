#include <Arduino.h>
#line 1 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>                             // Menggunakan library untuk LCD dengan modul I2C

LiquidCrystal_I2C lcd(0x27, 16, 2);                        // Inisialisasi LCD I2C dengan alamat 0x27 dan ukuran 16x2

#define detect_distance 30                                 // jarak (cm) minimal agar pembacaan ultrasonik dianggap valid
#define room_capacity 5                                    // jumlah maksimal orang dalam ruangan
#define flag_timeout 7000                                  // timeout reset flag1 dan 2 jika tidak ada orang masuk/keluar

#define reset_btn 6                                        // LCD reset button

#define e_s1 3                                             // Ultrasonic echo pin 1 terhubung ke pin 3 Arduino 
#define t_s1 4                                             // Ultrasonic trigger pin 1 terhubung ke pin 4 Arduino 

#define e_s2 10                                            // Ultrasonic echo pin 2 terhubung ke pin 10 Arduino 
#define t_s2 9                                             // Ultrasonic trigger pin 2 terhubung ke pin 9 Arduino 

#define relay 12                                           // Relay pin terhubung ke pin 12 Arduino

unsigned long dis_a = 0, dis_b = 0;                        // Variabel untuk menyimpan nilai dari sensor ultrasonic
unsigned long last_flag_set[2] = {0,0};
bool flag[2] = {false, false};                             // Variabel flag untuk sensor ultrasonic
bool led_on = false;                                       // apakah LED on?
int people_count = 0;                                      // Variabel untuk menyimpan jumlah orang


// Fungsi untuk membaca nilai ultrasonic
#line 28 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
void ultra_read(int pin_t, int pin_e, unsigned long &ultra_dist);
#line 51 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
void display_count();
#line 96 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
void reset_flags();
#line 106 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
void check_person_in();
#line 131 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
void check_person_out();
#line 154 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
void dev_display();
#line 185 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
void final_display();
#line 203 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
void update_led();
#line 217 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
void check_flag_timeout();
#line 231 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
void handle_reset_btn();
#line 238 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
void setup();
#line 258 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
void loop();
#line 28 "/home/yudhis/Documents/Kuliah/Embed/proyek/mas_sani/mas_sani.ino"
void ultra_read(int pin_t, int pin_e, unsigned long &ultra_dist){ 
  long time;

  // pastikan pin_t (trigger) clear
  digitalWrite(pin_t, LOW);
  delayMicroseconds(2);

  // trigger gelombang ultrasonik
  digitalWrite(pin_t, HIGH); // buat pin_t HIGH
  delayMicroseconds(10); // buat agar pin_t tetap HIGH selama 10us
  digitalWrite(pin_t, LOW);

  // baca durasi HIGH di pin_e (echo)
  time = pulseIn(pin_e, HIGH);

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
  delay(1);
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
  delay(d);
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

  if (dis_b < detect_distance && !flag[1]) {
    flag[1] = true;
    last_flag_set[1] = millis();

    if (flag[0]) {
      reset_flags();
      if (people_count < room_capacity) {
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

  if (dis_a < detect_distance && !flag[0]) {
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

  if (people_count == room_capacity) {
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
    digitalWrite(relay, HIGH);
    led_on = true;
  } else if (people_count == 0 && led_on) {
    digitalWrite(relay, LOW);
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
      if (millis() - last_flag_set[i] > flag_timeout) {
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
  if (!digitalRead(reset_btn)) {
    lcd.begin(16, 2);
    lcd.backlight();
  }
}

void setup(){
  Serial.begin(9600);                                      // Inisialisasi komunikasi serial pada 9600 bps
  while (!Serial);                                         // pastikan serial bener2 siap

  pinMode(reset_btn, INPUT_PULLUP);
  pinMode(relay, OUTPUT);                                  // Set pin relay sebagai output
  pinMode(e_s1, INPUT);
  pinMode(e_s2, INPUT);
  pinMode(t_s1, OUTPUT);
  pinMode(t_s2, OUTPUT);

  lcd.init();
  lcd.begin(16,2);                                         // Inisialisasi LCD
  lcd.backlight();                                         // Nyalakan backlight LCD
  lcd.setCursor(0, 0);
  lcd.print("     Welcome    ");
  Serial.print("hello");
  delay(1000);                                             // Tunggu sebentar
}

void loop(){ 
  handle_reset_btn(); 

  ultra_read(t_s1, e_s1, dis_a); delay(10);                // Membaca nilai sensor ultrasonic 1
  ultra_read(t_s2, e_s2, dis_b); delay(10);                // Membaca nilai sensor ultrasonic 2

  check_person_in();
  check_person_out();
  check_flag_timeout();

  // final_display();
  update_led();
  dev_display();
  delay(100);                                                // Tambahkan delay untuk memperbarui tampilan LCD
}
