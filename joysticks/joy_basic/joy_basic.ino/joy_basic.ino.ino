
#define JOY1_X 35 // derecha
#define JOY1_Y 34
#define JOY1_SW 32
#define JOY2_X 39 // izquierda
#define JOY2_Y 36
#define JOY2_SW 33

void setup() {
  Serial.begin(115200);
  
  pinMode(JOY1_SW, INPUT_PULLUP);
  pinMode(JOY2_SW, INPUT_PULLUP);
}

void loop() {
  int joy1_x = analogRead(JOY1_X);
  int joy1_y = analogRead(JOY1_Y);
  int joy1_sw = digitalRead(JOY1_SW);

  int joy2_x = analogRead(JOY2_X);
  int joy2_y = analogRead(JOY2_Y);
  int joy2_sw = digitalRead(JOY2_SW);

  Serial.print("Joystick 1 - X: "); Serial.print(joy1_x);
  Serial.print(" | Y: "); Serial.print(joy1_y);
  Serial.print(" | SW: "); Serial.print(joy1_sw);
  Serial.print("Joystick 2 - X: "); Serial.print(joy2_x);
  Serial.print(" | Y: "); Serial.print(joy2_y);
  Serial.print(" | SW: "); Serial.println(joy2_sw);

  delay(500);
}
