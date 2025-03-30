#include <ESP32Servo.h>

#define ESC_PIN 15
Servo esc;

void setup() {
  Serial.begin(115200);
  esc.attach(ESC_PIN, 1000, 2000);  // Rango estándar (ajustar si es necesario)
  
  Serial.println("Iniciando ESC...");
  esc.writeMicroseconds(1000);  // Señal de STOP
  delay(5000);  // Espera extendida para inicialización (algunos ESCs necesitan más tiempo)
  
  Serial.println("Enviando señal de arranque...");
  esc.writeMicroseconds(1200);  // Señal de arranque suave (ajusta según tu ESC)
  delay(2000);
}

void loop() {
  // Prueba subiendo y bajando la velocidad
  for (int speed = 1100; speed <= 2000; speed += 50) {  // Rango seguro para prueba
    esc.writeMicroseconds(speed);
    Serial.print("Velocidad: ");
    Serial.println(speed);
    delay(500);
  }
  Serial.print("stop");
  esc.writeMicroseconds(1000);  // Vuelve a STOP
  delay(1000);
}