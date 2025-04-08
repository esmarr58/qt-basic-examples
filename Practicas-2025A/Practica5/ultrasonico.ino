const int trigPin = 5;  // Pin de trigger
const int echoPin = 4;  // Pin de echo


void ultrasonicoSetup(){
    // Configurar los pines del HC-SR04
  pinMode(trigPin, OUTPUT); // El pin de trigger es salida
  pinMode(echoPin, INPUT);  // El pin de echo es entrada
  
}

float medirDistancia() {
  // Enviar un pulso corto al pin de trigger
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Medir la duración del pulso en el pin de echo
  long duration = pulseIn(echoPin, HIGH);

  // Calcular la distancia en centímetros
  // La velocidad del sonido es 343 m/s o 0.0343 cm/μs
  // La distancia es ida y vuelta, por lo que se divide entre 2
  float distance = duration * 0.0343 / 2;

  return distance;
}
