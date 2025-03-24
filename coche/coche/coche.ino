#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// Definición de los pines de los servos
int pinServo2 = 19; // derecha delante
int pinServo4 = 22; // derecha atras
int pinServo1 = 18; // izquierda delante
int pinServo3 = 21; // izquierda atras

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

// Estructura para recibir los datos del mando
typedef struct struct_message {
  int joy1_x;
  int joy1_y;
  int joy1_sw;
  int joy2_x;
  int joy2_y;
  int joy2_sw;
} struct_message;

struct_message myData;

// Margen para considerar que el joystick está en el centro
const int MARGIN = 10;

// Función de callback que se ejecuta cuando se reciben datos
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Datos Recibidos: ");
  Serial.print("\tJOY1_X: "); Serial.print(myData.joy1_x);
  Serial.print("\tJOY1_Y: "); Serial.print(myData.joy1_y);
  Serial.print("\tJOY1_SW: "); Serial.println(myData.joy1_sw);
}

void setup() {
  // Inicializar el monitor serie
  Serial.begin(115200);

  // Configurar el dispositivo como una estación Wi-Fi
  WiFi.mode(WIFI_STA);

  // Inicializar ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Registrar la función de callback para recibir datos
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  // Inicializar los servos
  servo1.attach(pinServo1);
  servo2.attach(pinServo2);
  servo3.attach(pinServo3);
  servo4.attach(pinServo4);

  // Parar el coche al inicio
  parar();

  Serial.println("----------------------------------------------------------");
  Serial.println("SOY EL COCHE, MI MAC-ADDRESS ES:");
  readMacAddress();
  Serial.println("----------------------------------------------------------");
  Serial.println("ESP-NOW ComunicationOneWay: Esperando datos...");

}

void loop() {
  // Controlar el coche según los valores recibidos del joystick
  if (myData.joy1_x > 100 - MARGIN) {
    alante();
    Serial.println("-- ALANTE --");
  } else if (myData.joy1_x < -100 + MARGIN) {
    atras();
    Serial.println("-- ATRAS --");
  } else if (myData.joy1_y > 100 - MARGIN) {
    izq();
    Serial.println("-- IZQ --");
  } else if (myData.joy1_y < -100 + MARGIN) {
    der();
    Serial.println("-- DER --");
  } else {
    parar();
    //Serial.println("-- STOP --");
  }
}

void alante() {
  int pos1 = 180;
  int pos2 = 0;
  servo1.write(pos1);
  servo2.write(pos2);
  servo3.write(pos1);
  servo4.write(pos2);
}

void atras() {
  int pos1 = 0;
  int pos2 = 180;
  servo1.write(pos1);
  servo2.write(pos2);
  servo3.write(pos1);
  servo4.write(pos2);
}

void izq() {
  int pos1 = 0;
  int pos2 = 180;
  servo1.write(pos1);
  servo2.write(pos1);
  servo3.write(pos1);
  servo4.write(pos1);
}

void der() {
  int pos1 = 0;
  int pos2 = 180;
  servo1.write(pos2);
  servo2.write(pos2);
  servo3.write(pos2);
  servo4.write(pos2);
}

void parar() {
  int posNeutral = 90; 
  servo1.write(posNeutral);
  servo2.write(posNeutral);
  servo3.write(posNeutral);
  servo4.write(posNeutral);
}

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}
