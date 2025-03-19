# Calibración Joysticks Analógicos con ESP32  

## 📌 Descripción  
Este programa permite leer y calibrar los valores obtenidos de **joysticks analógicos tipo PS2** conectados a una **placa ESP32 Dev Module**. 

## 🛠️ Materiales  
- **ESP32 DevKit V1** + Base de expansión  
- **2 Joysticks analógicos** (tipo PS2)  

<p align="center">
  <img src="joy01.jpg" alt="ESP32 Pines" width="45%">
  <img src="joy02.jpg" alt="Joystick" width="45%">
</p>

## 🔌 Diagrama de Conexiones  
Cada joystick cuenta con 5 pines principales:  

| **Pin del Joystick 1** | **ESP32** | **Pin del Joystick 2** | **ESP32** |
| ---------------------- | --------- | ---------------------- | --------- |
| **VCC** | 3.3V o 5V | **VCC** | 3.3V o 5V |
| **GND** | GND | **GND** | GND | GND |
| **VRX** | GPIO 34 | **VRX** | GPIO 36 |
| **VRY** | GPIO 35 | **VRY** | GPIO 39 |
| **SW** |  GPIO 32 | **SW** | GPIO 33 | 

> **Nota:** Los pines 34, 35, 36 y 39 del ESP32 son **de solo entrada (INPUT_ONLY)** y solo pueden leer señales analógicas.

## ⚙️ Funcionamiento  
1. Se leen las posiciones **X** e **Y** de ambos joysticks mediante los pines ADC.  
2. Se detectan las pulsaciones de los botones utilizando pines GPIO digitales.  
3. Se imprimen los valores obtenidos en el monitor serie para su análisis.  

## 🎯 Calibración de Joysticks  
### 📌 **Valores esperados**  
En teoría, las lecturas de `analogRead()` deben estar entre **0 y 2000**, y el punto de reposo debería ser **1000**.  

Sin embargo, en la práctica, estos valores varían. Ejemplo de lecturas en reposo:  
```
Joystick 1 - X: 937 | Y: 903 | SW: 1 Joystick 2 - X: 882 | Y: 939 | SW: 1
Joystick 1 - X: 930 | Y: 898 | SW: 1 Joystick 2 - X: 880 | Y: 942 | SW: 1
Joystick 1 - X: 933 | Y: 896 | SW: 1 Joystick 2 - X: 880 | Y: 942 | SW: 1
Joystick 1 - X: 928 | Y: 901 | SW: 1 Joystick 2 - X: 880 | Y: 940 | SW: 1
```
### 🔧 **Proceso de calibración**  
1. **Calibración del punto central**  
   - Se toman múltiples lecturas del joystick en reposo para calcular el **valor promedio** de cada eje.  
   - Este dato servirá como referencia para ajustar futuras mediciones.  

1.1 Durante la calibración, se toman numLecturas (por ejemplo, 1000) lecturas de cada eje (X e Y) para ambos joysticks. Estas lecturas se suman en las variables:
```
sum_joy1_x: Suma de las lecturas del eje X del joystick derecho.
sum_joy1_y: Suma de las lecturas del eje Y del joystick derecho.
sum_joy2_x: Suma de las lecturas del eje X del joystick izquierdo.
sum_joy2_y: Suma de las lecturas del eje Y del joystick izquierdo.
```

1.2. Cálculo del valor promedio:
Una vez que se han tomado todas las lecturas, se calcula el valor promedio para cada eje dividiendo la suma total por el número de lecturas (numLecturas):
```
joy1_x_center = sum_joy1_x / numLecturas; // Valor central del eje X del joystick derecho
joy1_y_center = sum_joy1_y / numLecturas; // Valor central del eje Y del joystick derecho
joy2_x_center = sum_joy2_x / numLecturas; // Valor central del eje X del joystick izquierdo
joy2_y_center = sum_joy2_y / numLecturas; // Valor central del eje Y del joystick izquierdo
```
1.3. Uso del valor central:
Una vez calculados los valores centrales, se utilizan para ajustar las lecturas futuras. Por ejemplo, para el eje X del joystick derecho:
```
int valor_ajustado = analogRead(JOY1_X) - joy1_x_center;
```

Esto asegura que, cuando el joystick esté en reposo, el valor ajustado sea cercano a 0.


2. **Calibración de los ejes X e Y**  
   - Se solicita al usuario mover los joysticks a los extremos (izquierda/derecha para **X**, arriba/abajo para **Y**).  
   - Con estas mediciones, se calcula un **factor de escala** que normaliza las lecturas.  

3. **Filtrado y ajuste de valores**  
   - Los valores se ajustan a un rango de **-100 a 100**.  
   - Se implementa un filtro para eliminar pequeñas variaciones y mejorar la precisión.  

🚀 **¡Listo para calibrar tus joysticks con ESP32!** 🎮