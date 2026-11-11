#include <Arduino.h>

// Motor / vacuum pump output. Active-high; driven directly from a 3.3V GPIO.
const int MOTOR_PIN = 5;

// EV release / evac solenoid. Active-high; driven directly from a 3.3V GPIO.
const int EV_PIN = 6;

// P-SW vacuum / pressure switch to ground, read with the internal pull-up.
const int PSW_PIN = 16;

// Lid (SAFE / S-SW) switch to ground, read with the internal pull-up.
const int SAFE_PIN = 17;

// Onboard RGB LED on the ESP32-S3-N16R8 DevKitC-1.
const int RGB_PIN = 48;

// Switch must hold this long before a trip is confirmed.
const unsigned long VAC_DEBOUNCE_MS = 250;

// Keep pumping after a confirmed trip so the bag can stabilize.
const unsigned long VAC_EXTRA_MS = 5000;

// Wait after the pump stops before opening the release valve.
const unsigned long EV_DELAY_MS = 3000;

// How long the release valve stays open.
const unsigned long EV_OPEN_MS = 2000;

// Hard stop so the pump cannot run forever if something goes wrong.
const unsigned long PUMP_MAX_MS = 20000;

unsigned long vacSince = 0;   // when the switch first read "vacuum"
unsigned long vacTripAt = 0;  // when the trip was confirmed (0 = not yet)
unsigned long evArmAt = 0;    // when the pump stopped (0 = not armed)
unsigned long evOpenAt = 0;   // when the valve opened (0 = closed)

bool pumpOn = false;
unsigned long pumpStart = 0;

bool lidClosed() {
  return digitalRead(SAFE_PIN) == LOW;  // pressed / closed = LOW
}

bool vacuumReached() {
  return digitalRead(PSW_PIN) == LOW;  // flip to HIGH if backwards
}

void setValve(bool open) {
  digitalWrite(EV_PIN, open ? HIGH : LOW);
  Serial.println(open ? ">> Valve OPEN" : ">> Valve CLOSED");
}

void setPump(bool on) {
  if (on && !lidClosed()) {
    Serial.println(">> Lid open, pump blocked");
    on = false;
  }
  bool wasOn = pumpOn;
  pumpOn = on;
  digitalWrite(MOTOR_PIN, on ? HIGH : LOW);
  if (on) {
    pumpStart = millis();
    vacSince = 0;
    vacTripAt = 0;
    evArmAt = 0;
    if (evOpenAt) {
      evOpenAt = 0;
      setValve(false);
    }
  } else if (wasOn && evArmAt == 0 && evOpenAt == 0) {
    evArmAt = millis();
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
  pinMode(PSW_PIN, INPUT_PULLUP);
  pinMode(EV_PIN, OUTPUT);
  digitalWrite(EV_PIN, LOW);
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

  // Debounced vacuum trip, then a top-off period
  if (pumpOn) {
    if (vacuumReached()) {
      if (vacSince == 0) {
        vacSince = millis();
      }
      if (vacTripAt == 0 && millis() - vacSince >= VAC_DEBOUNCE_MS) {
        vacTripAt = millis();
        Serial.println(">> Vacuum reached, topping off");
      }
    } else {
      vacSince = 0;  // flicker, reset debounce
    }

    if (vacTripAt && millis() - vacTripAt >= VAC_EXTRA_MS) {
      Serial.println(">> Top-off done");
      setPump(false);
    }
  }

  // Safety timeout
  if (pumpOn && millis() - pumpStart > PUMP_MAX_MS) {
    Serial.println(">> Pump timeout");
    setPump(false);
  }

  // Release valve timing
  if (evArmAt && millis() - evArmAt >= EV_DELAY_MS) {
    evArmAt = 0;
    evOpenAt = millis();
    setValve(true);
  }
  if (evOpenAt && millis() - evOpenAt >= EV_OPEN_MS) {
    evOpenAt = 0;
    setValve(false);
  }

  updateLed(lid);

  // Status every half second
  static unsigned long last = 0;
  if (millis() - last >= 500) {
    last = millis();
    Serial.printf("LID=%s  PUMP=%d  VAC=%d\n",
                  lid ? "closed" : "open", pumpOn, vacuumReached());
  }
}
