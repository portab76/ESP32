#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// ¡¡¡ REMPLACE CON LA MAC-ADDRESS DEL COCCHE !!!
uint8_t broadcastAddress[] = {0x3C,0x8A,0x1F,0x08,0x8A,0x3C};

// Structure to send data
typedef struct struct_message {
  int joy1_x;
  int joy1_y;
  int joy1_sw;
  int joy2_x;
  int joy2_y;
  int joy2_sw;
} struct_message;
// Create a struct_message called myData
struct_message myData;
esp_now_peer_info_t peerInfo;


#define JOY1_X 35 // derecha
#define JOY1_Y 34
#define JOY1_SW 32
#define JOY2_X 39 // izquierda
#define JOY2_Y 36
#define JOY2_SW 33

// Variables para almacenar los valores de calibración
int joy1_x_center = 0;
int joy1_y_center = 0;
int joy2_x_center = 0;
int joy2_y_center = 0;

// Factores de escala proporcionalidad para los ejes X e Y
float joy1_x_scale = 1.0;
float joy2_x_scale = 1.0;
float joy1_y_scale = 1.0;
float joy2_y_scale = 1.0;

// Rango esperado de los joysticks (ajusta según tus joysticks)
const int joy_min = 0;
const int joy_max = 2000;

// Rango deseado de salida
const int output_min = -100;
const int output_max = 100;
const int lecturas_calibracion = 250;

void setup() {
  Serial.begin(115200);
  
  // Init ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }  

  Serial.println("----------------------------------------------------------");
  Serial.println("SOY EL MANDO. MI MAC_ADDRESS ES:");
  readMacAddress();
  // Imprimir la dirección MAC
  Serial.println("EL COCHE TIENE LA MAC_ADDRESS:");
  for (int i = 0; i < 6; i++) {
    if (broadcastAddress[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(broadcastAddress[i], HEX);
    if (i < 5) {
      Serial.print(":");
    }
  }
  Serial.println("\t");
  Serial.println("----------------------------------------------------------");

  pinMode(JOY1_SW, INPUT_PULLUP);
  pinMode(JOY2_SW, INPUT_PULLUP);
  calibrarPuntoCentral(lecturas_calibracion); 
  calibrarEjeX(lecturas_calibracion); 
  calibrarEjeY(lecturas_calibracion);

}

void loop() {
  // Leer y ajustar los valores de los joysticks
  int joy1_x = ajustarEjeX(analogRead(JOY1_X), joy1_x_center, joy1_x_scale, joy_min, joy_max, output_min, output_max);
  int joy1_y = ajustarEjeY(analogRead(JOY1_Y), joy1_y_center, joy1_y_scale, joy_min, joy_max, output_min, output_max);
  int joy1_sw = digitalRead(JOY1_SW);

  int joy2_x = ajustarEjeX(analogRead(JOY2_X), joy2_x_center, joy2_x_scale, joy_min, joy_max, output_min, output_max);
  int joy2_y = ajustarEjeY(analogRead(JOY2_Y), joy2_y_center, joy2_y_scale, joy_min, joy_max, output_min, output_max);
  int joy2_sw = digitalRead(JOY2_SW);

 // Asignar los valores a la estructura
  myData.joy1_x = joy1_x;
  myData.joy1_y = joy1_y;
  myData.joy1_sw = joy1_sw;
  myData.joy2_x = joy2_x;
  myData.joy2_y = joy2_y;
  myData.joy2_sw = joy2_sw;

  // Enviar los datos a través de ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  if (result == ESP_OK) {
      Serial.print("Datos enviados: ");
      Serial.print("\tJOY2_X ");  Serial.print(joy2_x);
      Serial.print("\tJOY2_Y ");  Serial.print(joy2_y);
      Serial.print("\tJOY2_SW "); Serial.print(joy2_sw);
      Serial.print("\tJOY1_X: "); Serial.print(joy1_x);
      Serial.print("\tJOY1_Y ");  Serial.print(joy1_y);
      Serial.print("\tJOY1_SW "); Serial.println(joy1_sw);
  } else {
    Serial.println("Error al enviar los datos");
  }  
  delay(100);
}
 
// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  bool statusSend = status == ESP_NOW_SEND_SUCCESS ? true : false;
}

// Funcin para calibrar el punto central
void calibrarPuntoCentral(int numLecturas) {
  long sum_joy1_x = 0, sum_joy1_y = 0, sum_joy2_x = 0, sum_joy2_y = 0;

  Serial.println("Calibrando punto central... Manten las palancas en reposo.");

  for (int i = 0; i < numLecturas; i++) {
    sum_joy1_x += analogRead(JOY1_X);
    sum_joy1_y += analogRead(JOY1_Y);
    sum_joy2_x += analogRead(JOY2_X);
    sum_joy2_y += analogRead(JOY2_Y);

    if ((i + 1) % (numLecturas / 10) == 0) {
      int progreso = ((i + 1) * 100) / numLecturas;
      Serial.print(" ");
      Serial.print(progreso);
      Serial.print("%");
    }
    delay(10); 
  }

  joy1_x_center = sum_joy1_x / numLecturas;
  joy1_y_center = sum_joy1_y / numLecturas;
  joy2_x_center = sum_joy2_x / numLecturas;
  joy2_y_center = sum_joy2_y / numLecturas;

  Serial.println("Calibración del punto central completada:");
  Serial.print("Joystick_DER - X: "); Serial.print(joy1_x_center);
  Serial.print("\tY: "); Serial.println(joy1_y_center);
  Serial.print("Joystick_IZQ - X: "); Serial.print(joy2_x_center);
  Serial.print("\tY: "); Serial.println(joy2_y_center);
}

// Funcin para calibrar el eje Y (hacia arriba)
void calibrarEjeY(int numLecturas) {
  Serial.println("Mueve ambas palancas hacia arriba para calibrar el eje Y.");

  // Esperar a que ambos ejes Y estén por debajo de -85
  while (true) {
    int joy1_y = ajustarValor(analogRead(JOY1_Y), joy1_y_center, joy_min, joy_max, output_min, output_max);
    int joy2_y = ajustarValor(analogRead(JOY2_Y), joy2_y_center, joy_min, joy_max, output_min, output_max);

    if (joy1_y < -65 && joy2_y < -65) {
      break; // Salir del bucle cuando ambos ejes Y estén por debajo de -85
    }

    delay(100); // Pequeño delay para no saturar el bucle
  }

  Serial.println("Comenzando calibración del eje Y...");

  long sum_joy1_y = 0, sum_joy2_y = 0;

  for (int i = 0; i < numLecturas; i++) {
    sum_joy1_y += analogRead(JOY1_Y);
    sum_joy2_y += analogRead(JOY2_Y);

    // Mostrar el progreso cada 10%
    if ((i + 1) % (numLecturas / 10) == 0) {
      int progreso = ((i + 1) * 100) / numLecturas;
      Serial.print(" ");
      Serial.print(progreso);
      Serial.print("%");
    }

    delay(10); // Pequeño delay entre lecturas
  }

  // Calcular el valor medio cuando las palancas están hacia arriba
  int joy1_y_min = sum_joy1_y / numLecturas;
  int joy2_y_min = sum_joy2_y / numLecturas;

  // Calcular la proporcionalidad
  joy1_y_scale = (float)output_min / ajustarValor(joy1_y_min, joy1_y_center, joy_min, joy_max, output_min, output_max);
  joy2_y_scale = (float)output_min / ajustarValor(joy2_y_min, joy2_y_center, joy_min, joy_max, output_min, output_max);

  Serial.println("Calibración del eje Y completada:");
  Serial.print("Joystick_DER - Y (min): "); Serial.print(joy1_y_min);
  Serial.print("\tFactor de escala: "); Serial.println(joy1_y_scale, 4);
  Serial.print("Joystick_IZQ - Y (min): "); Serial.print(joy2_y_min);
  Serial.print("\tFactor de escala: "); Serial.println(joy2_y_scale, 4);
}

// Función para calibrar el eje X (hacia la derecha, valores negativos)
void calibrarEjeX(int numLecturas) {
  Serial.println("Mueve ambas palancas hacia la derecha para calibrar el eje X.");
  // Esperar a que ambos ejes X estén por debajo de -85 (valores negativos)
  while (true) {
    int joy1_x = ajustarValor(analogRead(JOY1_X), joy1_x_center, joy_min, joy_max, output_min, output_max);
    int joy2_x = ajustarValor(analogRead(JOY2_X), joy2_x_center, joy_min, joy_max, output_min, output_max);

    if (joy1_x < -65 && joy2_x < -65) {
      break; // Salir del bucle cuando ambos ejes X estén por debajo de -85
    }
    //Serial.print("\tEje 1 X ");Serial.print(joy1_x); Serial.print("\tEje 2 X ");Serial.println(joy2_x);
    delay(100); // Pequeño delay para no saturar el bucle
  }

  Serial.println("Comenzando calibración del eje X...");

  long sum_joy1_x = 0, sum_joy2_x = 0;

  for (int i = 0; i < numLecturas; i++) {
    sum_joy1_x += analogRead(JOY1_X);
    sum_joy2_x += analogRead(JOY2_X);

    // Mostrar el progreso cada 10%
    if ((i + 1) % (numLecturas / 10) == 0) {
      int progreso = ((i + 1) * 100) / numLecturas;
      Serial.print(" ");
      Serial.print(progreso);
      Serial.print("%");
    }

    delay(10); // Pequeño delay entre lecturas
  }

  // Calcular el valor medio cuando las palancas están hacia la derecha (valores negativos)
  int joy1_x_min = sum_joy1_x / numLecturas;
  int joy2_x_min = sum_joy2_x / numLecturas;

  // Calcular la proporcionalidad
  joy1_x_scale = (float)output_min / ajustarValor(joy1_x_min, joy1_x_center, joy_min, joy_max, output_min, output_max);
  joy2_x_scale = (float)output_min / ajustarValor(joy2_x_min, joy2_x_center, joy_min, joy_max, output_min, output_max);

  Serial.println("Calibración del eje X completada:");
  Serial.print("Joystick_DER - X (min): "); Serial.print(joy1_x_min);
  Serial.print("\tFactor de escala: "); Serial.println(joy1_x_scale, 4);
  Serial.print("Joystick_IZQ - X (min): "); Serial.print(joy2_x_min);
  Serial.print("\tFactor de escala: "); Serial.println(joy2_x_scale, 4);
}

// Funcin para ajustar el valor del joystick al rango deseado
int ajustarValor(int valor, int centro, int min_entrada, int max_entrada, int min_salida, int max_salida) {
  // Ajustar el valor al centro
  int valor_ajustado = valor - centro;

  // Escalar el valor al rango deseado
  int rango_entrada = max_entrada - min_entrada;
  int rango_salida = max_salida - min_salida;

  int valor_escalado = map(valor_ajustado, -rango_entrada / 2, rango_entrada / 2, min_salida, max_salida);

  // Limitar el valor al rango de salida
  valor_escalado = constrain(valor_escalado, min_salida, max_salida);

  return valor_escalado;
}

// Funcin para ajustar el valor del eje X con proporcionalidad
int ajustarEjeX(int valor, int centro, float escala, int min_entrada, int max_entrada, int min_salida, int max_salida) {
  // Ajustar el valor al centro
  int valor_ajustado = valor - centro;

  // Escalar el valor al rango deseado
  int rango_entrada = max_entrada - min_entrada;
  int rango_salida = max_salida - min_salida;

  int valor_escalado = map(valor_ajustado, -rango_entrada / 2, rango_entrada / 2, min_salida, max_salida);

  // Aplicar la proporcionalidad
  valor_escalado = (int)(valor_escalado * escala);

  // Limitar el valor al rango de salida
  valor_escalado = constrain(valor_escalado, min_salida, max_salida);

  if (valor_escalado >= -10 && valor_escalado <= 10) {
    valor_escalado = 0;
  }

  return valor_escalado;
}

// Funcin para ajustar el valor del eje Y con proporcionalidad
int ajustarEjeY(int valor, int centro, float escala, int min_entrada, int max_entrada, int min_salida, int max_salida) {
  // Ajustar el valor al centro
  int valor_ajustado = valor - centro;

  // Escalar el valor al rango deseado
  int rango_entrada = max_entrada - min_entrada;
  int rango_salida = max_salida - min_salida;

  int valor_escalado = map(valor_ajustado, -rango_entrada / 2, rango_entrada / 2, min_salida, max_salida);

  // Aplicar la proporcionalidad
  valor_escalado = (int)(valor_escalado * escala);

  // Limitar el valor al rango de salida
  valor_escalado = constrain(valor_escalado, min_salida, max_salida);

  if (valor_escalado >= -10 && valor_escalado <= 10) {
    valor_escalado = 0;
  }

  return valor_escalado;
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

