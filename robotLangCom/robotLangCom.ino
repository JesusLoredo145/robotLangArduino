// === RobotLang -> Servomotores ===
// Comandos esperados (por línea):
// HOME
// MOVE HOMBRO 95
// MOVE CODO 80
// MOVE MUNECA 100
// WAIT 500
// GRIP ABRIR
// GRIP CERRAR
//
// NOTA:
// - El interprete normalmente manda ángulos absolutos.
//   Ejemplo: si RobotLang dice Mover(HOMBRO, 5), C# puede convertirlo a MOVE HOMBRO 95.
// - Este sketch mueve lentamente el servo hasta el ángulo recibido.
// - Acepta MUNECA y MUÑECA.
//
// RECOMENDACIÓN ELÉCTRICA:
// - Usa fuente externa de 5V para servos.
// - Une GND de la fuente con GND del Arduino.

#include <Arduino.h>
#include <Servo.h>

// -------------------------
// Pines de señal de servos
// -------------------------
const int PIN_MUNECA = 5;
const int PIN_HOMBRO = 6;
const int PIN_CODO   = 7;

// -------------------------
// Configuración de velocidad
// -------------------------
// Mientras más alto sea este valor, más lento se mueve.
const int SERVO_STEP_DELAY_MS = 25;   // velocidad lenta y segura
const int SERVO_STEP_DEGREES  = 1;    // mover de 1 grado por paso

// Límites por articulación (ajústalos si tu mecánica lo necesita)
const int MIN_MUNECA = 0;
const int MAX_MUNECA = 180;

const int MIN_HOMBRO = 0;
const int MAX_HOMBRO = 180;

const int MIN_CODO = 0;
const int MAX_CODO = 180;

// Posición HOME
const int HOME_MUNECA = 90;
const int HOME_HOMBRO = 90;
const int HOME_CODO   = 90;

// Objetos servo
Servo servoMuneca;
Servo servoHombro;
Servo servoCodo;

// Estado actual de ángulos
int currentMuneca = HOME_MUNECA;
int currentHombro = HOME_HOMBRO;
int currentCodo   = HOME_CODO;

String line;

// -------------------------------------
// Utilidades
// -------------------------------------
bool startsWithWord(const String& s, const char* w) {
  return s.startsWith(w);
}

int clampValue(int value, int minValue, int maxValue) {
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}

String normalizeJoint(String joint) {
  joint.trim();
  joint.toUpperCase();

  // Normalización manual de variantes comunes
  if (joint == "MUÑECA") return "MUNECA";
  return joint;
}

// -------------------------------------
// Movimiento lento de servos
// -------------------------------------
void moveServoSlow(Servo& servo, int& currentAngle, int targetAngle) {
  targetAngle = clampValue(targetAngle, 0, 180);

  if (targetAngle == currentAngle) {
    servo.write(currentAngle);
    return;
  }

  if (targetAngle > currentAngle) {
    for (int pos = currentAngle; pos <= targetAngle; pos += SERVO_STEP_DEGREES) {
      servo.write(pos);
      delay(SERVO_STEP_DELAY_MS);
    }
  } else {
    for (int pos = currentAngle; pos >= targetAngle; pos -= SERVO_STEP_DEGREES) {
      servo.write(pos);
      delay(SERVO_STEP_DELAY_MS);
    }
  }

  currentAngle = targetAngle;
  servo.write(currentAngle);
}

// -------------------------------------
// HOME
// -------------------------------------
void goHomeSlow() {
  moveServoSlow(servoMuneca, currentMuneca, HOME_MUNECA);
  moveServoSlow(servoHombro, currentHombro, HOME_HOMBRO);
  moveServoSlow(servoCodo,   currentCodo,   HOME_CODO);
}

// -------------------------------------
// Movimiento por articulación
// -------------------------------------
bool moveJointByName(const String& rawJoint, int angle) {
  String joint = normalizeJoint(rawJoint);

  if (joint == "HOMBRO") {
    angle = clampValue(angle, MIN_HOMBRO, MAX_HOMBRO);
    moveServoSlow(servoHombro, currentHombro, angle);
    return true;
  }

  if (joint == "CODO") {
    angle = clampValue(angle, MIN_CODO, MAX_CODO);
    moveServoSlow(servoCodo, currentCodo, angle);
    return true;
  }

  if (joint == "MUNECA") {
    angle = clampValue(angle, MIN_MUNECA, MAX_MUNECA);
    moveServoSlow(servoMuneca, currentMuneca, angle);
    return true;
  }

  return false;
}

// -------------------------------------
// Setup
// -------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) { /* compatibilidad */ }

  // Adjuntar servos
  servoMuneca.attach(PIN_MUNECA);
  servoHombro.attach(PIN_HOMBRO);
  servoCodo.attach(PIN_CODO);

  // Llevar a HOME inicial lentamente
  servoMuneca.write(currentMuneca);
  servoHombro.write(currentHombro);
  servoCodo.write(currentCodo);

  delay(500);

  Serial.println("OK");
}

// -------------------------------------
// Loop principal
// -------------------------------------
void loop() {
  if (!Serial.available()) return;

  line = Serial.readStringUntil('\n');
  line.trim();

  if (line.length() == 0) return;

  // HOME
  if (line == "HOME") {
    goHomeSlow();
    Serial.println("OK");
    return;
  }

  // WAIT <ms>
  if (startsWithWord(line, "WAIT ")) {
    int ms = line.substring(5).toInt();
    if (ms < 0) ms = 0;
    delay((unsigned long)ms);
    Serial.println("OK");
    return;
  }

  // GRIP <accion>
  // Por ahora solo respondemos OK para no romper la comunicación.
  // Más adelante aquí puedes conectar un servo de pinza.
  if (startsWithWord(line, "GRIP ")) {
    Serial.println("OK");
    return;
  }

  // MOVE <JOINT> <ANGLE>
  if (startsWithWord(line, "MOVE ")) {
    int firstSpace = line.indexOf(' ');
    int secondSpace = line.indexOf(' ', firstSpace + 1);

    if (secondSpace < 0) {
      Serial.println("ERR BAD MOVE FORMAT");
      return;
    }

    String joint = line.substring(firstSpace + 1, secondSpace);
    joint.trim();

    String angleStr = line.substring(secondSpace + 1);
    angleStr.trim();

    // Validación simple del ángulo
    bool hasDigits = false;
    for (unsigned int i = 0; i < angleStr.length(); i++) {
      char c = angleStr.charAt(i);
      if (isDigit(c) || c == '-') {
        hasDigits = true;
      } else {
        hasDigits = false;
        break;
      }
    }

    if (!hasDigits) {
      Serial.println("ERR BAD ANGLE");
      return;
    }

    int angle = angleStr.toInt();

    if (angle < 0 || angle > 180) {
      Serial.println("ERR ANGLE OUT OF RANGE");
      return;
    }

    bool ok = moveJointByName(joint, angle);
    if (!ok) {
      Serial.print("ERR UNKNOWN JOINT ");
      Serial.println(joint);
      return;
    }

    Serial.println("OK");
    return;
  }

  Serial.println("ERR UNKNOWN COMMAND");
}
