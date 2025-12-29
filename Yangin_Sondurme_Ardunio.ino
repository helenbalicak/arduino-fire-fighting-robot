#include <Servo.h>

// --- PARÇALAR ---
Servo gozcuServo;   // Radar
Servo silahServo;   // Hortum

// --- PİNLER ---
const int GOZCU_PIN = 2;    
const int SILAH_PIN = 3;    
const int POMPA_PIN = 6;    
const int ALEV_PIN  = A0;  // Ateş Sensörü
const int SU_PIN    = A1;  // YENİ: Su Seviye Sensörü

// Trafik Işıkları
const int LED_KIRMIZI = 11; 
const int LED_SARI    = 12; 
const int LED_YESIL   = 13; 

// --- AYARLAR ---
int taramaHizi = 30;
int hassasiyet = 400;      // Ateş algılama sınırı
int suSiniri   = 100;      // Su sensörü ıslakken 300-500 verir, kuruysa 0-50.

// Röle Tetikleme
#define POMPA_AC    LOW
#define POMPA_KAPAT HIGH

void setup() {
  gozcuServo.attach(GOZCU_PIN);
  silahServo.attach(SILAH_PIN);

  pinMode(POMPA_PIN, OUTPUT);
  pinMode(ALEV_PIN, INPUT);
  pinMode(SU_PIN, INPUT);    // Su sensörü giriş
  
  pinMode(LED_KIRMIZI, OUTPUT);
  pinMode(LED_SARI, OUTPUT);
  pinMode(LED_YESIL, OUTPUT);

  digitalWrite(POMPA_PIN, POMPA_KAPAT); 

  // Başlangıç Selamlaması
  digitalWrite(LED_KIRMIZI, HIGH); delay(100); digitalWrite(LED_KIRMIZI, LOW);
  digitalWrite(LED_SARI, HIGH);    delay(100); digitalWrite(LED_SARI, LOW);
  digitalWrite(LED_YESIL, HIGH);   delay(100); digitalWrite(LED_YESIL, LOW);

  gozcuServo.write(90);
  silahServo.write(90);
  delay(1000);
}

void loop() {
  // Önce Su Kontrolü Yap!
  if (suVarMi() == false) {
    // SU BİTTİ UYARISI (Bütün ışıklar yanıp söner)
    digitalWrite(LED_KIRMIZI, HIGH);
    digitalWrite(LED_SARI, HIGH);
    digitalWrite(LED_YESIL, HIGH);
    delay(200);
    digitalWrite(LED_KIRMIZI, LOW);
    digitalWrite(LED_SARI, LOW);
    digitalWrite(LED_YESIL, LOW);
    delay(200);
    return; // Su yoksa aşağıya inme, tarama yapma!
  }

  // Su varsa normal devriyeye devam et (Yeşil Yan)
  digitalWrite(LED_YESIL, HIGH);

  // --- SOLA TARAMA ---
  for (int aci = 20; aci <= 160; aci += 2) {
    gozcuServo.write(aci);
    delay(taramaHizi);
    if (atesVarMi()) ateseSaldir(aci);
  }

  // --- SAĞA TARAMA ---
  for (int aci = 160; aci >= 20; aci -= 2) {
    gozcuServo.write(aci);
    delay(taramaHizi);
    if (atesVarMi()) ateseSaldir(aci);
  }
}

// --- FONKSİYONLAR ---

bool suVarMi() {
  int seviye = analogRead(SU_PIN);
  if (seviye > suSiniri) return true; // Su var
  else return false;                  // Kuru
}

bool atesVarMi() {
  if (analogRead(ALEV_PIN) < hassasiyet) return true;
  else return false;
}

void ateseSaldir(int hedefAci) {
  // Ateş Bulundu!
  digitalWrite(LED_YESIL, LOW);
  digitalWrite(LED_SARI, HIGH); // Nişan al
  
  silahServo.write(hedefAci);
  delay(600); 

  // Saldırı
  digitalWrite(LED_SARI, LOW);
  digitalWrite(LED_KIRMIZI, HIGH);
  digitalWrite(POMPA_PIN, POMPA_AC);

  while (analogRead(ALEV_PIN) < hassasiyet) {
    // Sıkarken su biterse pompayı korumak için dur!
    if (suVarMi() == false) {
      digitalWrite(POMPA_PIN, POMPA_KAPAT);
      break; 
    }
    delay(100);
  }

  digitalWrite(POMPA_PIN, POMPA_KAPAT);
  digitalWrite(LED_KIRMIZI, LOW);
  digitalWrite(LED_YESIL, HIGH);
  
  silahServo.write(90);
  delay(500);
}