## Control de ESC con ESP32 ##
Este programa permite controlar un ESC (Electronic Speed Controller) utilizando un microcontrolador ESP32 y la biblioteca ESP32Servo.

**Requisitos**
Placa ESP32
ESC compatible con señal PWM
Fuente de alimentación adecuada para el motor/ESC
Biblioteca ESP32Servo instalada en el IDE de Arduino

**Conexiones**
Conecta el ESC al pin GPIO15 del ESP32

```
ESC MOTOR
ROJO      + BATERIA
NEGRO     GND ESP32
BLANCO    GPIO ESP32
GRIS      GND (luces de momento siempre encendidas)

BATERIA
+ SERVO
- GND ESP32 + - ESC
```

# Funcionamiento #

El programa realiza lo siguiente:

**Inicialización:**
Configura el ESC con un rango estándar de 1000-2000μs
Envía una señal de STOP (1000μs) durante 5 segundos (necesario para inicializar muchos ESCs)

**Secuencia de arranque:**
Envía una señal de arranque suave (1200μs) durante 2 segundos

**Bucle principal:**
Incrementa gradualmente la velocidad desde 1100μs hasta 2000μs en pasos de 50μs
Cada cambio de velocidad se mantiene durante 500ms
Vuelve a la posición de STOP (1000μs) al final del ciclo
Repite el proceso indefinidamente

**Ajustes importantes**
El rango de señales (1000-2000μs) puede necesitar ajuste según tu ESC específico
El tiempo de inicialización (5 segundos) puede variar según el modelo de ESC
La velocidad de arranque (1200μs) puede necesitar ajuste
