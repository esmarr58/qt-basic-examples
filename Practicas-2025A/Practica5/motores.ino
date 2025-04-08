// Definir pines de control para los motores
const int motor1PinA = 10; // Pin de control 1 para Motor 1
const int motor1PinB = 9; // Pin de control 2 para Motor 1

const int motor2PinA = 11; // Pin de control 1 para Motor 2
const int motor2PinB = 12;  // Pin de control 2 para Motor 2



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


// Función para detener/frenar los motores
void frenarMotores() {
  Serial.println("Freno");
  // Para frenar, establecer ambos pines A y B en HIGH
  digitalWrite(motor1PinA, LOW);
  digitalWrite(motor1PinB, LOW);
  digitalWrite(motor2PinA, LOW);
  digitalWrite(motor2PinB, LOW);


}

void setupMotores() {
  // Inicializar los pines de los motores como salida
  pinMode(motor1PinA, OUTPUT);
  pinMode(motor1PinB, OUTPUT);

  pinMode(motor2PinA, OUTPUT);
  pinMode(motor2PinB, OUTPUT);




  
}
