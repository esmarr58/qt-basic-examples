#define TRIG_PIN 12 // Pin de TRIG conectado al HC-SR04
#define ECHO_PIN 13 // Pin de ECHO conectado al HC-SR04

void SensorDistanciaSetup() {
  pinMode(TRIG_PIN, OUTPUT); // Configura el pin TRIG como salida
  pinMode(ECHO_PIN, INPUT);  // Configura el pin ECHO como entrada
}

float medicion(){
  long duration;
  float distance;

  // Envía un pulso ultrasónico
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Mide el tiempo del pulso de regreso
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calcula la distancia en centímetros
  distance = (duration * 0.034) / 2;
  return distance;
}
