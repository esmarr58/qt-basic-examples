// Definir pines de control para los motores
const int motor1PinA = 12; // Pin de control 1 para Motor 1
const int motor1PinB = 11; // Pin de control 2 para Motor 1
const int motor1PWM = 4;   // Pin PWM para Motor 1

const int motor2PinA = 10; // Pin de control 1 para Motor 2
const int motor2PinB = 9;  // Pin de control 2 para Motor 2
const int motor2PWM = 5;   // Pin PWM para Motor 2



// Función para configurar los pines de control de un motor
void establecerDireccionMotor(int pinA, int pinB, bool adelante) {
  if (adelante) {
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, LOW);
  } else {
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, HIGH);
  }
}

// Función para establecer la dirección de ambos motores hacia adelante
void moverAdelante() {
  Serial.println("OK adelante");
  establecerDireccionMotor(motor1PinA, motor1PinB, true); // Motor 1 adelante
  establecerDireccionMotor(motor2PinA, motor2PinB, true); // Motor 2 adelante
}

// Función para establecer la dirección de ambos motores hacia atrás
void moverAtras() {
  Serial.println("OK atras");
  establecerDireccionMotor(motor1PinA, motor1PinB, false); // Motor 1 atrás
  establecerDireccionMotor(motor2PinA, motor2PinB, false); // Motor 2 atrás
}

// Función para girar a la izquierda
void girarIzquierda() {
  Serial.println("OK izquierda");
  establecerDireccionMotor(motor1PinA, motor1PinB, false); // Motor 1 atrás
  establecerDireccionMotor(motor2PinA, motor2PinB, true);  // Motor 2 adelante
}

// Función para girar a la derecha
void girarDerecha() {
  Serial.println("OK derecha");
  establecerDireccionMotor(motor1PinA, motor1PinB, true);  // Motor 1 adelante
  establecerDireccionMotor(motor2PinA, motor2PinB, false); // Motor 2 atrás
}

// Función independiente para controlar la velocidad de ambos motores
void establecerVelocidad(int velocidad) {
  // Convertir el valor de 0-100 a 0-255 (resolución de 8 bits)
  int valorPWM = map(velocidad, 0, 100, 0, 255);

  analogWrite(motor1PWM, valorPWM);
  analogWrite(motor1PWM, valorPWM);
}

// Función para detener/frenar los motores
void frenarMotores() {
  Serial.println("Freno");
  // Para frenar, establecer ambos pines A y B en HIGH
  digitalWrite(motor1PinA, HIGH);
  digitalWrite(motor1PinB, HIGH);
  digitalWrite(motor2PinA, HIGH);
  digitalWrite(motor2PinB, HIGH);

  // Detener el PWM usando ledcWrite en los canales correctos
  analogWrite(motor1PWM, 0);
  analogWrite(motor1PWM, 0);
}

void setupMotores() {
  // Inicializar los pines de los motores como salida
  pinMode(motor1PinA, OUTPUT);
  pinMode(motor1PinB, OUTPUT);
  pinMode(motor1PWM, OUTPUT);

  pinMode(motor2PinA, OUTPUT);
  pinMode(motor2PinB, OUTPUT);
  pinMode(motor2PWM, OUTPUT);

  pinMode(motor1PWM, OUTPUT);
  pinMode(motor2PWM, OUTPUT);
  analogWrite(motor1PWM, 0);
  analogWrite(motor1PWM, 0);

  
}
