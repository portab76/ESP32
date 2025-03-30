#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Configuración del GPS
HardwareSerial SerialGPS(1);  // UART2 (RX=GPIO16, TX=GPIO17)
TinyGPSPlus gps;

// Configuración BLE
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHUNK_SIZE 20  // Tamaño máximo de cada fragmento BLE
#define DELIMITER "\n---\n"  // Delimitador entre mensajes JSON

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Dispositivo BLE conectado");
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Dispositivo BLE desconectado");
      pServer->startAdvertising();
    }
};

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("=== Drone GPS con BLE ===");

  BLEDevice::init("DroneGPS_BLE");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  BLEDevice::startAdvertising();
  Serial.println("BLE listo. Busca 'DroneGPS_BLE' en tu dispositivo.");
}

void sendInChunks(const String &data) {
  if (!deviceConnected) return;
  
  // Primero enviamos el delimitador
  sendChunk(DELIMITER);
  
  // Luego enviamos los datos JSON en chunks
  for (unsigned int i = 0; i < data.length(); i += CHUNK_SIZE) {
    unsigned int endIndex = (i + CHUNK_SIZE < data.length()) ? i + CHUNK_SIZE : data.length();
    sendChunk(data.substring(i, endIndex));
  }
}

void sendChunk(const String &chunk) {
  pCharacteristic->setValue(chunk.c_str());
  pCharacteristic->notify();
  delay(10); // Pequeña pausa entre chunks
  Serial.print("Enviado: ");
  Serial.println(chunk);
}

String formatGPSData() {
  String data = "{";
  
  if (gps.location.isValid()) {
    data += "\"la\":" + String(gps.location.lat(), 6) + ",";
    data += "\"lo\":" + String(gps.location.lng(), 6) + ",";
    data += "\"al\":" + String(gps.altitude.meters()) + ",";
  } else {
    data += "\"loc\":\"NaN\",";
  }

  if (gps.speed.isValid()) {
    data += "\"sp\":" + String(gps.speed.kmph()) + ",";
  } else {
    data += "\"sp\":\"NaN\",";
  }

  if (gps.satellites.isValid()) {
    data += "\"sat\":" + String(gps.satellites.value());
  } else {
    data += "\"sat\":0";
  }

  data += "}";
  return data;
}

void loop() {
  while (SerialGPS.available() > 0) {
    if (gps.encode(SerialGPS.read())) {
      String gpsData = formatGPSData();
      Serial.print("JSON completo: ");
      Serial.println(gpsData);
      sendInChunks(gpsData);
    }
  }
}