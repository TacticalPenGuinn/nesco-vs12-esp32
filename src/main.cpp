#include <Arduino.h>

// Motor / vacuum pump output. Active-high; driven directly from a 3.3V GPIO.
const int MOTOR_PIN = 5;

// Hard stop so the pump cannot run forever if something goes wrong.
const unsigned long PUMP_MAX_MS = 20000;

bool pumpOn = false;
unsigned long pumpStart = 0;

void setPump(bool on) {
  pumpOn = on;
  digitalWrite(MOTOR_PIN, on ? HIGH : LOW);
  if (on) {
    pumpStart = millis();
  }
  Serial.println(on ? ">> Pump ON" : ">> Pump OFF");
}

void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  delay(500);
  Serial.println("Type 'm' to toggle pump");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'm' || c == 'M') {
      setPump(!pumpOn);
    }
  }

  // Safety timeout
  if (pumpOn && millis() - pumpStart > PUMP_MAX_MS) {
    Serial.println(">> Pump timeout");
    setPump(false);
  }

  // Status every half second
  static unsigned long last = 0;
  if (millis() - last >= 500) {
    last = millis();
    Serial.printf("PUMP=%d\n", pumpOn);
  }
}
