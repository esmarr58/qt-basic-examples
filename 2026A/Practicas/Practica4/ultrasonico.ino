// ==========================================
// HC-SR04 + ESP32
// ECHO  -> GPIO 5
// TRIG  -> GPIO 6
// ==========================================

#define TRIG_PIN 6
#define ECHO_PIN 5

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("Prueba HC-SR04");
}

void loop() {

  // Asegurar trigger en LOW
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Pulso de 10 us al trigger
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Leer duración del eco
  long duracion = pulseIn(ECHO_PIN, HIGH);

  // Calcular distancia en cm
  float distancia = duracion * 0.0343 / 2;

  // Mostrar resultado
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  delay(500);
}
