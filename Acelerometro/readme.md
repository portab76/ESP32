# Visualización 3D con MPU6050

## Descripción del Proyecto
Sistema para visualizar en tiempo real los datos de un sensor MPU6050 (acelerómetro y giroscopio) mediante una representación 3D en navegador web, incluyendo:

1. **Firmware ESP32**: Lectura de datos del sensor y transmisión vía WebSocket
2. **Interfaz Web**:
   - `cubo.html`: Visualización básica con cubo 3D interactivo
   - `recibir.html`: Dashboard avanzado con gráficos y mapas
3. **Algoritmos**: Fusión de datos de acelerómetro y giroscopio para estimación precisa de orientación

## Configuración Hardware
```
ESP32 GPIO21 (SDA) → MPU6050 SDA
ESP32 GPIO22 (SCL) → MPU6050 SCL
ESP32 3.3V         → MPU6050 VCC
ESP32 GND          → MPU6050 GND
```
Configuración Software

- Arduino IDE con soporte ESP32
- Bibliotecas:
  - Wire.h
  - WiFi.h
  - WebServer.h
  - WebSocketsServer.h
  - ArduinoJson.h
  - math.h

##Estructura del Proyecto
```
Acelerometro/
├── MPU6050/
│   └── MPU6050.ino       # Código principal para ESP32
└── WebClient/
    ├── cubo.html         # Visualización 3D básica
    ├── recibir.html      # Dashboard avanzado
    └── img/           #
```


## Cálculos Matemáticos
**Conversión de unidades:**
```
// Acelerómetro (LSB → g)
accX = rawX / 16384.0;

// Giroscopio (LSB → °/s)
gyroX = rawGX / 131.0;
```
**Obtención de ángulos:**
```
// Desde acelerómetro
pitch = atan2(-accX, sqrt(accY² + accZ²)) * (180/PI);
roll = atan2(accY, accZ) * (180/PI);

// Desde giroscopio (integración)
angleX += (gyroX - offsetX) * deltaTime;
```

**Fusión de sensores:**

```
// Filtro complementario (α=0.98)
angleX = α*(angleX + gyroX*Δt) + (1-α)*accelAngleX;
```

## Formato de Datos
El ESP32 envía datos en formato JSON:

```
{
    "acelerometro": {"x":0.12, "y":-0.05, "z":0.98},
    "giroscopio": {"x":1.25, "y":-0.75, "z":0.32}
}
```

## Uso
 
 - Cargar firmware al ESP32. 
   - Configurar credenciales WiFi 
   - Anota la IP que se muestra apor la consola del COM
 
 - Abrir interfaz web (cubo.html o recibir.html)
 - Actualizar IP en los archivos HTML