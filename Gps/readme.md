# GPS Monitor - Aplicación Android con BLE

Aplicación Android para monitorear datos GPS a través de Bluetooth Low Energy (BLE), conectándose a un dispositivo ESP32.

## Características principales

- Conexión BLE con dispositivo ESP32
- Recepción y procesamiento de datos GPS en formato JSON
- Visualización del GPS.
- Mecanismo robusto para manejar mensajes fragmentados BLE
- Sistema de delimitadores para identificar mensajes completos

## Componentes del sistema

### Aplicación Android
- Escanea y conecta con dispositivo BLE llamado "DroneGPS_BLE"
- Implementa lógica para reconstruir mensajes JSON fragmentados
- Procesa datos GPS con el formato:
```json
  {
    "la": 12.345678,  // Latitud
    "lo": -12.345678, // Longitud
    "al": 150.5,      // Altitud en metros
    "sp": 25.3,       // Velocidad en km/h
    "sat": 8          // Satélites visibles
  }
```
**Firmware ESP32**
Lee datos GPS a través de UART
Formatea datos en JSON compacto
Transmite datos por BLE en chunks con delimitadores

**Iplementa servidor BLE con:**
UUID de servicio: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
UUID de característica: beb5483e-36e1-4688-b7f5-ea07361b26a8

**Permisos:**
BLUETOOTH
BLUETOOTH_ADMIN
BLUETOOTH_CONNECT
BLUETOOTH_SCAN
ACCESS_FINE_LOCATION

## Para el firmware ESP32 ##

Módulo GPS conectado a UART (pines GPIO16/RX, GPIO17/TX)
Biblioteca TinyGPS++ para procesamiento de datos GPS
Biblioteca BLE de ESP32

**Configuración**
Cargar el firmware en el ESP32
Conectar el módulo GPS a los pines correctos
Compilar y ejecutar la aplicación Android
Buscar y conectar con el dispositivo "DroneGPS_BLE"

**Mecanismo de comunicación**
El ESP32 envía datos con el formato json
La app Android detecta los delimitadores --- para identificar mensajes completos
Los mensajes JSON pueden dividirse en múltiples chunks BLE
La app reensambla los chunks antes del procesamiento
