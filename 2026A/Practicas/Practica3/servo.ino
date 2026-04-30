
#include <ESP32Servo.h>
Servo miServo;        // Crear objeto tipo Servo
const int pinServo = 14;  // Pin donde está conectado el servo
void setup() {
   pinMode(pinServo, OUTPUT);
   miServo.setPeriodHertz(50);          // Frecuencia típica de los servos
   miServo.attach(pinServo, 500, 2400); // Pin, pulso mínimo y máximo en microsegundos
   miServo.write(0);  
   


}

void loop() {
  int pos = 0;
    for (pos = 0; pos <= 180; pos += 1) { // sweep from 0 degrees to 180 degrees
         miServo.write(pos);  
        delay(50);
    }


 



}
