// PISTOLA COMO MOUSE Y TECLADO (versión simple de prueba_mouse, hace lo mismo)
// Placa en el IDE: "Arduino Leonardo".
// Arranca en pausa. START: toque corto = activa o pausa el control.
//                         mantener 1 segundo = recalibra (con la pistola apoyada y quieta).
//   Mover la pistola = mouse     Joystick = W A S D     Click del joystick = Shift
//   RB = click izquierdo         LB = click derecho y mira más lenta
//   Y = R (recargar)             B = C (agacharse)
#include <Wire.h>
#include <Mouse.h>
#include <Keyboard.h>

// Pines
const int pinRB = 5, pinLB = 9, pinY = 8, pinB = 7, pinStart = 6, pinClick = A1;
const int pinJoyX = A3, pinJoyY = A2;

// Ajustes
const float sens = 0.002;     // velocidad de la mira (con LB va a la mitad)
const int zonaMuerta = 250;   // giros más chicos se ignoran (evita temblor)
const int umbral = 200;       // cuánto empujar el joystick para caminar

long offH = 0, offV = 0;      // "cero" del giroscopio, se mide al enchufar
float giroH = 0, giroV = 0;   // giro suavizado
float restoX = 0, restoY = 0; // decimales del mouse que quedan para la vuelta siguiente
bool activo = false, startAntes = false;
unsigned long startDesde = 0; // cuándo se apretó START
bool recalibrado = false;     // ya se recalibró en este apretón de START
bool teclaW, teclaA, teclaS, teclaD, teclaShift, teclaR, teclaC, clickIzq, clickDer;

// Lee el giro vertical (eje Y del MPU) y horizontal (eje Z). El MPU va acostado en
// el cañón con la flecha X hacia la punta.
bool leerGiro(int &h, int &v) {
  Wire.beginTransmission(0x68);
  Wire.write(0x45);                 // registro GYRO_Y (salteamos GYRO_X)
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(0x68, 4) < 4) return false;
  byte yAlto = Wire.read();
  byte yBajo = Wire.read();
  byte zAlto = Wire.read();
  byte zBajo = Wire.read();
  v = (int16_t)(yAlto << 8 | yBajo);
  h = (int16_t)(zAlto << 8 | zBajo);
  return true;
}

bool apretado(int pin) {
  return digitalRead(pin) == LOW;   // con INPUT_PULLUP, apretado = LOW
}

// Avisan a la PC solo cuando una tecla o un click cambia
void tecla(uint8_t k, bool ahora, bool &antes) {
  if (ahora && !antes) Keyboard.press(k);
  if (!ahora && antes) Keyboard.release(k);
  antes = ahora;
}

void click(uint8_t b, bool ahora, bool &antes) {
  if (ahora && !antes) Mouse.press(b);
  if (!ahora && antes) Mouse.release(b);
  antes = ahora;
}

// Mide el "cero" del giroscopio: promedio de 100 lecturas con la pistola quieta.
// El cero se corre cuando el sensor se calienta, por eso se puede repetir con START.
void calibrar() {
  long sumaH = 0, sumaV = 0;
  int n = 0;
  for (int i = 0; i < 100; i++) {
    int h, v;
    if (leerGiro(h, v)) {
      sumaH += h;
      sumaV += v;
      n++;
    }
    delay(5);
  }
  if (n > 0) {
    offH = sumaH / n;
    offV = sumaV / n;
  }
  giroH = 0;
  giroV = 0;
  restoX = 0;
  restoY = 0;
}

void setup() {
  Wire.begin();
  Wire.setClock(100000);
  Wire.setWireTimeout(3000, true);
  Wire.beginTransmission(0x68);     // despertar el MPU
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();
  delay(100);

  pinMode(pinRB, INPUT_PULLUP);
  pinMode(pinLB, INPUT_PULLUP);
  pinMode(pinY, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  pinMode(pinStart, INPUT_PULLUP);
  pinMode(pinClick, INPUT_PULLUP);

  calibrar();

  Mouse.begin();
  Keyboard.begin();
}

void loop() {
  // START: toque corto = activar o pausar; mantener 1 segundo = recalibrar
  bool start = apretado(pinStart);
  if (start && !startAntes) {       // recién apretado
    startDesde = millis();
    recalibrado = false;
  }
  if (start && !recalibrado && millis() - startDesde >= 1000) {
    calibrar();
    recalibrado = true;
  }
  if (!start && startAntes) {       // recién soltado
    // más de 30 ms para ignorar los rebotes del botón
    if (!recalibrado && millis() - startDesde > 30) activo = !activo;
  }
  startAntes = start;

  // Pistola -> mouse
  int h, v;
  if (leerGiro(h, v)) {
    giroH = giroH * 0.7 + (h - offH) * 0.3;   // suavizado
    giroV = giroV * 0.7 + (v - offV) * 0.3;
  }
  float s = apretado(pinLB) ? sens / 2 : sens;
  if (activo) {
    if (abs(giroH) > zonaMuerta) restoX -= giroH * s;
    if (abs(giroV) > zonaMuerta) restoY += giroV * s;
    int x = constrain((int)restoX, -127, 127);
    int y = constrain((int)restoY, -127, 127);
    restoX -= (int)restoX;
    restoY -= (int)restoY;
    if (x != 0 || y != 0) Mouse.move(x, y);
  }

  // Joystick -> W A S D. Va girado: el eje X del módulo es adelante/atrás
  // (adelante = lado opuesto a los pines) y el eje Y es derecha/izquierda.
  int adelante = analogRead(pinJoyX) - 512;
  int derecha = analogRead(pinJoyY) - 512;
  tecla('w', activo && adelante > umbral, teclaW);
  tecla('s', activo && adelante < -umbral, teclaS);
  tecla('d', activo && derecha > umbral, teclaD);
  tecla('a', activo && derecha < -umbral, teclaA);

  // Botones
  tecla(KEY_LEFT_SHIFT, activo && apretado(pinClick), teclaShift);
  tecla('r', activo && apretado(pinY), teclaR);
  tecla('c', activo && apretado(pinB), teclaC);
  click(MOUSE_LEFT, activo && apretado(pinRB), clickIzq);
  click(MOUSE_RIGHT, activo && apretado(pinLB), clickDer);

  delay(5);
}
