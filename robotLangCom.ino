//primeras pruebas sin servos 
String line = "";

void setup() {
  Serial.begin(115200);
  while(!Serial) { ; }
  Serial.println("OK READY");
}

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      line.trim();
      if (line.length() > 0) {
        handleLine(line);
      }
      line = "";
    } else {
      line += c;
    }
  }
}

void handleLine(String cmd) {
  cmd.trim();

  if (cmd == "HOME") {
    Serial.println("OK");
    return;
  }

  if (cmd.startsWith("WAIT ")) {
    Serial.println("OK");
    return;
  }

  if (cmd.startsWith("MOVE ")) {
    // MOVE <JOINT> <ANGLE>
    int p1 = cmd.indexOf(' ');
    int p2 = cmd.indexOf(' ', p1 + 1);
    if (p2 < 0) { Serial.println("ERR BAD MOVE FORMAT"); return; }

    String joint = cmd.substring(p1 + 1, p2);
    String angS  = cmd.substring(p2 + 1);
    joint.trim();
    angS.trim();

    // Normaliza a MAYÚSCULAS
    joint.toUpperCase();

    // Normaliza MUÑECA -> MUNECA (por encoding)
    joint.replace("Ñ", "N");

    int angle = angS.toInt();
    if (angle < 0 || angle > 180) { Serial.println("ERR ANGLE RANGE"); return; }

    if (!(joint == "HOMBRO" || joint == "CODO" || joint == "MUNECA")) {
      Serial.println("ERR UNKNOWN JOINT");
      return;
    }

    // Fase 1: solo ACK
    Serial.println("OK");
    return;
  }

  Serial.println("ERR UNKNOWN CMD");
}
