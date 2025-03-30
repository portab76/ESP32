#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <math.h>

// Variables para almacenar el vector de gravedad calibrado
float gravityX = 0;
float gravityY = 0;
float gravityZ = 0;

// Declaraciones
void setupMPU();
void connectWiFi();
void readMPUData(int16_t &AcX, int16_t &AcY, int16_t &AcZ, int16_t &GyX, int16_t &GyY, int16_t &GyZ);
void handleSensorData();
void sendSensorData();

const char* ssid = "tu_SSID";
const char* password = "tu_PASSWORD.";
WebServer server(80);
WebSocketsServer webSocket(81);

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA=21, SCL=22
  setupMPU();
  connectWiFi();
  
  server.on("/sensor", HTTP_GET, handleSensorData);
  server.begin();
  webSocket.begin();
  webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_CONNECTED) Serial.printf("Cliente #%u conectado\n", num);
  });
  calibrateGravity();
}

void loop() {
  server.handleClient();
  webSocket.loop();
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 100) {
    sendSensorData();
    lastSend = millis();
  }
}

void setupMPU() {
  Wire.beginTransmission(0x68);
  Wire.write(0x6B); // PWR_MGMT_1
  Wire.write(0);
  Wire.endTransmission(true);
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");
  Serial.println("IP address is : ");
  Serial.println(WiFi.localIP());
}

void readMPUData(int16_t &AcX, int16_t &AcY, int16_t &AcZ, int16_t &GyX, int16_t &GyY, int16_t &GyZ) {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 14, true);
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read(); // Saltar temperatura
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}

// Función para enviar datos por WebSocket
void sendSensorData() {
  int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;
  readMPUData(AcX, AcY, AcZ, GyX, GyY, GyZ); // Lee datos del MPU6050


    // Convertir a unidades físicas
  float accX, accY, accZ, gyroX_dps, gyroY_dps, gyroZ_dps;
  
  // Conversión para acelerómetro (±2g -> 16384 LSB/g)
  accX = AcX / 16384.0;
  accY = AcY / 16384.0;
  accZ = AcZ / 16384.0;
  
// Eliminar gravedad
  removeGravity(accX, accY, accZ);


  // Conversión para giroscopio (±250°/s -> 131 LSB/°/s)
  gyroX_dps = GyX / 131.0;
  gyroY_dps = GyY / 131.0;
  gyroZ_dps = GyZ / 131.0;

  // Crea un JSON con los datos
  DynamicJsonDocument doc(200);
  doc["acelerometro"]["x"] = accX;
  doc["acelerometro"]["y"] = accY;
  doc["acelerometro"]["z"] = accZ;
  doc["giroscopio"]["x"] = gyroX_dps;
  doc["giroscopio"]["y"] = gyroY_dps;
  doc["giroscopio"]["z"] = gyroZ_dps;

  // Convierte a String y envía por WebSocket
  String jsonStr;
  serializeJson(doc, jsonStr);
  webSocket.broadcastTXT(jsonStr);
}

// Función para manejar peticiones HTTP (API REST)
void handleSensorData() {
  int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;
  readMPUData(AcX, AcY, AcZ, GyX, GyY, GyZ);

  DynamicJsonDocument doc(200);
  doc["acelerometro"]["x"] = AcX;
  doc["acelerometro"]["y"] = AcY;
  doc["acelerometro"]["z"] = AcZ;
  doc["giroscopio"]["x"] = GyX;
  doc["giroscopio"]["y"] = GyY;
  doc["giroscopio"]["z"] = GyZ;

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}

// Calibración inicial (ejecutar cuando el sensor está en reposo)
void calibrateGravity() {
  int16_t AcX, AcY, AcZ;
  // Leer múltiples muestras y promediar
  for (int i = 0; i < 100; i++) {
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 6, true);
    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();
    gravityX += AcX / 16384.0;
    gravityY += AcY / 16384.0;
    gravityZ += AcZ / 16384.0;
    delay(10);
  }
  gravityX /= 100;
  gravityY /= 100;
  gravityZ /= 100;
  Serial.println("Calibración completada. Vector de gravedad:");
  Serial.print(gravityX, 4); Serial.print("g, ");
  Serial.print(gravityY, 4); Serial.print("g, ");
  Serial.println(gravityZ, 4); Serial.println("g");
}

// Elimina la gravedad de las lecturas
void removeGravity(float &accX, float &accY, float &accZ) {
  accX -= gravityX;
  accY -= gravityY;
  accZ -= gravityZ;
}