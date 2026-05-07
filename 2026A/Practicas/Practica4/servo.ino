#include <ESP32Servo.h> //version 3.0.9

Servo miServo;        // Crear objeto tipo Servo
const int pinServo = 35;  // Pin donde está conectado el servo
int angulo = 0;         // Ángulo inicial


void setup() {
  // put your setup code here, to run once:
   miServo.setPeriodHertz(50);          // Frecuencia típica de los servos
   miServo.attach(pinServo, 500, 2400); // Pin, pulso mínimo y máximo en microsegundos
   miServo.write(angulo);  

}

void loop() {
  // put your main code here, to run repeatedly:
  for(int angulo = 0; angulo <= 180; angulo +=10){
       miServo.write(angulo);
       delay(300);
    
  }

}
