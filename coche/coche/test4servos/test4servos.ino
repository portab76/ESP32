#include <ESP32Servo.h>

int pinServo2 = 19; // derecha delante
int pinServo4 = 22; // derecha atras
int pinServo1 = 18; // izquierda delante
int pinServo3 = 21; // izquierda atras

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

const int DELAY = 1000; 

void setup() {
  servo1.attach(pinServo1);
  servo2.attach(pinServo2);
  servo3.attach(pinServo3);
  servo4.attach(pinServo4);
}

void loop() {
  alante(); delay(DELAY);
  atras();  delay(DELAY);
  izq();    delay(DELAY);
  der();    delay(DELAY);
  parar();  delay(DELAY);
}

void alante() {
  Serial.println("-- ALANTE --");
  int pos1 = 180;
  int pos2 = 0;
  servo1.write(pos1);
  servo2.write(pos2);
  servo3.write(pos1);
  servo4.write(pos2);
}

void atras() {
  Serial.println("-- ATRAS --");
  int pos1 = 0;
  int pos2 = 180;
  servo1.write(pos1);
  servo2.write(pos2);
  servo3.write(pos1);
  servo4.write(pos2);
}

void izq() {
  Serial.println("-- IZQUIERDA --");
  int pos1 = 0;
  int pos2 = 180;
  servo1.write(pos1);
  servo2.write(pos1);
  servo3.write(pos1);
  servo4.write(pos1);
}

void der() {
  Serial.println("-- DERECHA --");
  int pos1 = 0;
  int pos2 = 180;
  servo1.write(pos2);
  servo2.write(pos2);
  servo3.write(pos2);
  servo4.write(pos2);
}

void parar() {
  Serial.println("-- PARAR --");
  int posNeutral = 90; 
  servo1.write(posNeutral);
  servo2.write(posNeutral);
  servo3.write(posNeutral);
  servo4.write(posNeutral);
}