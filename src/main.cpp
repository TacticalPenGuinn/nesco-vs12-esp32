#include <Arduino.h>

// Motor / vacuum pump output. Active-high; driven directly from a 3.3V GPIO.
const int MOTOR_PIN = 5;

// Lid (SAFE / S-SW) switch to ground, read with the internal pull-up.
const int SAFE_PIN = 17;

// Onboard RGB LED on the ESP32-S3-N16R8 DevKitC-1.
const int RGB_PIN = 48;

// Hard stop so the pump cannot run forever if something goes wrong.
const unsigned long PUMP_MAX_MS = 20000;

bool pumpOn = false;
unsigned long pumpStart = 0;

bool lidClosed() {
  return digitalRead(SAFE_PIN) == LOW;  // pressed / closed = LOW
}

void setPump(bool on) {
  if (on && !lidClosed()) {
    Serial.println(">> Lid open, pump blocked");
    on = false;
  }
  pumpOn = on;
  digitalWrite(MOTOR_PIN, on ? HIGH : LOW);
  if (on) {
    pumpStart = millis();
  }
  Serial.println(on ? ">> Pump ON" : ">> Pump OFF");
}

void updateLed(bool lid) {
  static int lastState = -1;
  int state = !lid ? 0 : (pumpOn ? 2 : 1);
  if (state == lastState) {
    return;
  }
  lastState = state;
  switch (state) {
    case 0: neopixelWrite(RGB_PIN, 0, 0, 0);  break;  // off (lid open)
    case 1: neopixelWrite(RGB_PIN, 0, 40, 0); break;  // green (lid closed, idle)
    case 2: neopixelWrite(RGB_PIN, 0, 0, 40); break;  // blue (pumping)
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  pinMode(SAFE_PIN, INPUT_PULLUP);
  neopixelWrite(RGB_PIN, 0, 0, 0);
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

  bool lid = lidClosed();

  // Stop if the lid opens while running
  if (pumpOn && !lid) {
    Serial.println(">> Lid opened");
    setPump(false);
  }

  // Safety timeout
  if (pumpOn && millis() - pumpStart > PUMP_MAX_MS) {
    Serial.println(">> Pump timeout");
    setPump(false);
  }

  updateLed(lid);

  // Status every half second
  static unsigned long last = 0;
  if (millis() - last >= 500) {
    last = millis();
    Serial.printf("LID=%s  PUMP=%d\n",
                  lid ? "closed" : "open", pumpOn);
  }
}
