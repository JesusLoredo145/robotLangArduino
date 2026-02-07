// === Demo RobotLang -> LEDs ===
// Mapea articulaciones a LEDs para simular movimientos
// Comandos esperados (por línea):
// HOME
// MOVE BASE 100
// MOVE HOMBRO 85
// MOVE CODO 90
// WAIT 500
// GRIP ABRIR
// GRIP CERRAR

#include <Arduino.h>

const int LED_MUNECA   = 5;
const int LED_HOMBRO = 6;
const int LED_CODO   = 7;

String line;

void blinkPin(int pin, int msOn = 180, int msOff = 80, int times = 1) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(msOn);
    digitalWrite(pin, LOW);
    delay(msOff);
  }
}

void homeAnim() {
  // animación corta para "HOME"
  blinkPin(LED_MUNECA, 120, 60, 1);
  blinkPin(LED_HOMBRO, 120, 60, 1);
  blinkPin(LED_CODO, 120, 60, 1);
}

int jointToPin(const String& joint) {
  if (joint == "HOMBRO") return LED_HOMBRO;
  if (joint == "CODO") return LED_CODO;
  if (joint == "MUÑECA" || joint == "MUNECA") return LED_MUNECA;
  return -1;
}


bool startsWithWord(const String& s, const char* w) {
  return s.startsWith(w);
}

void setup() {
  pinMode(LED_MUNECA, OUTPUT);
  pinMode(LED_HOMBRO, OUTPUT);
  pinMode(LED_CODO, OUTPUT);

  digitalWrite(LED_MUNECA, LOW);
  digitalWrite(LED_HOMBRO, LOW);
  digitalWrite(LED_CODO, LOW);

  Serial.begin(9600);
  while (!Serial) { /* por compat */ }

  Serial.println("OK"); // opcional: saludo inicial
}

void loop() {
  if (!Serial.available()) return;

  line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;

  // Para depurar:
  // Serial.print("RX: "); Serial.println(line);

  if (line == "HOME") {
    homeAnim();
    Serial.println("OK");
    return;
  }

  if (startsWithWord(line, "WAIT ")) {
    int ms = line.substring(5).toInt();
    if (ms < 0) ms = 0;
    delay((unsigned long)ms);
    Serial.println("OK");
    return;
  }

  if (startsWithWord(line, "GRIP ")) {
    // demo: parpadeo de todos
    blinkPin(LED_MUNECA, 80, 40, 1);
    blinkPin(LED_HOMBRO, 80, 40, 1);
    blinkPin(LED_CODO, 80, 40, 1);
    Serial.println("OK");
    return;
  }

  if (startsWithWord(line, "MOVE ")) {
    // Formato: MOVE <JOINT> <ANGLE>
    // Ej: MOVE BASE 100
    int firstSpace = line.indexOf(' ');
    int secondSpace = line.indexOf(' ', firstSpace + 1);
    if (secondSpace < 0) {
      Serial.println("ERR BAD MOVE FORMAT");
      return;
    }

    String joint = line.substring(firstSpace + 1, secondSpace);
    joint.trim();
    joint.toUpperCase();

    String angleStr = line.substring(secondSpace + 1);
    angleStr.trim();
    int angle = angleStr.toInt(); // en demo no importa mucho

    int pin = jointToPin(joint);
    if (pin < 0) {
      Serial.print("ERR UNKNOWN JOINT ");
      Serial.println(joint);
      return;
    }

    // demo: parpadeo proporcional simple (opcional)
    // entre 1 y 3 parpadeos dependiendo del ángulo
    int times = 1;
    if (angle >= 120) times = 3;
    else if (angle >= 90) times = 2;

    blinkPin(pin, 180, 90, times);
    Serial.println("OK");
    return;
  }

  // Si llega algo no reconocido:
  Serial.println("ERR UNKNOWN COMMAND");
}
