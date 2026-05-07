// ==========================================
// Sensor Siguelineas 4 Canales + ESP32
//
// A1 -> GPIO 18
// A2 -> GPIO 17
// A3 -> GPIO 16
// A4 -> GPIO 15
// ==========================================

#define S1 18
#define S2 17
#define S3 16
#define S4 15

void setup() {
  Serial.begin(115200);

  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);

  Serial.println("Prueba Sensor Siguelineas 4 Canales");
}

void loop() {

  int a1 = digitalRead(S1);
  int a2 = digitalRead(S2);
  int a3 = digitalRead(S3);
  int a4 = digitalRead(S4);

  Serial.print("A1: ");
  Serial.print(a1);

  Serial.print(" | A2: ");
  Serial.print(a2);

  Serial.print(" | A3: ");
  Serial.print(a3);

  Serial.print(" | A4: ");
  Serial.println(a4);

  delay(200);
}
