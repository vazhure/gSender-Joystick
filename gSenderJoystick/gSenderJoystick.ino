#include <Joystick.h>
#include <Encoder.h>

#define BTN_DELAY 100  // ms
#define MODES_CNT 5    // WORK MODES COUNT: 0, X, Y, Z, A
#define PULSE_DIVIDER 2.5

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   2 + 2 * MODES_CNT + 3,  // Button Count
                   0,                      // Hat Switch Count
                   false, false, false,    // X and Y, but no Z Axis
                   false, false, false,    // No Rx, Ry, or Rz
                   false, false,           // No rudder or throttle
                   false, false, false);   // No accelerator, brake, or steering

Encoder SideEncoder(2, 3);

// USED PINS
// ENC A - D2
// ENC B - D3
// ENC BTN - D1
// ENA BTN - D0
// MODE 0 - D4
// MODE X - D5
// MODE Y - D6
// MODE Z - D7
// MODE A - D8
// BTN PREC - D9
// BTN NML - A3
// BTN FAST - A2
// BTN PLUS - A1
// BTN MINUS - A0

// Initialization
void setup() {
  for (int t = 0; t <= 21; t++)
    pinMode(t, INPUT_PULLUP);
  Joystick.begin();
  SideEncoder.write(0);
}

int32_t oldState = INT32_MAX;

unsigned long lastPulseTime = 0;
int prevMode = -1;

enum MODE { NONE = 1 << 4,
            XAXIS = 1 << 5,
            YAXIS = 1 << 6,
            ZAXIS = 1 << 7,
            AAXIS = 1 << 8 };

// MAIN PROGRAM CYCLE
void loop() {
  int32_t state = 0;
  for (int t = 0; t <= 9; t++)
    state |= !digitalRead(t) << t;

  state |= (!digitalRead(A3) != LOW) << 10;
  state |= (!digitalRead(A2) != LOW) << 11;
  state |= (!digitalRead(A1) != LOW) << 12;
  state |= (!digitalRead(A0) != LOW) << 13;

  int pulses = SideEncoder.read() / PULSE_DIVIDER;

  if (oldState != state || pulses != 0) {

    int oldEna = oldState & 1;
    int encBtn = state & (1 << 1);
    int ena = state & 1;
    int mode = state & (NONE | XAXIS | YAXIS | ZAXIS | AAXIS);
    int prec = state & (1 << 9);
    int nrm = state & (1 << 10);    // A3
    int fast = state & (1 << 11);   // A2
    int plus = state & (1 << 12);   // A1
    int minus = state & (1 << 13);  // A0

    bool bEnabled = ena > 0;

    if (prevMode != mode) {
      prevMode = mode;
      SideEncoder.write(0);
    }

    Joystick.setButton(0, bEnabled ? encBtn > 0 : 0);
    Joystick.setButton(1, bEnabled ? fast > 0 : 0);
    Joystick.setButton(2, bEnabled ? nrm > 0 : 0);
    Joystick.setButton(3, bEnabled ? prec > 0 : 0);

    plus = pulses == 0 ? plus : pulses > 0;
    minus = pulses == 0 ? minus : pulses < 0;

    if (pulses != 0) {
      unsigned long delta = millis() - lastPulseTime;
      if (delta < BTN_DELAY)
        delay(BTN_DELAY - delta);
    }

    Joystick.setButton(4, (bEnabled && (mode == NONE)) ? plus > 0 : 0);
    Joystick.setButton(5, (bEnabled && (mode == NONE)) ? minus > 0 : 0);
    Joystick.setButton(6, (bEnabled && (mode == XAXIS)) ? plus > 0 : 0);
    Joystick.setButton(7, (bEnabled && (mode == XAXIS)) ? minus > 0 : 0);
    Joystick.setButton(8, (bEnabled && (mode == YAXIS)) ? plus > 0 : 0);
    Joystick.setButton(9, (bEnabled && (mode == YAXIS)) ? minus > 0 : 0);
    Joystick.setButton(10, (bEnabled && (mode == ZAXIS)) ? plus > 0 : 0);
    Joystick.setButton(11, (bEnabled && (mode == ZAXIS)) ? minus > 0 : 0);
    Joystick.setButton(12, (bEnabled && (mode == AAXIS)) ? plus > 0 : 0);
    Joystick.setButton(13, (bEnabled && (mode == AAXIS)) ? minus > 0 : 0);

    if (pulses != 0) {
      delay(BTN_DELAY);
      lastPulseTime = millis();
      pulses > 0 ? pulses -= PULSE_DIVIDER : pulses += PULSE_DIVIDER;
      SideEncoder.write(pulses);
      for (int t = 4; t < 14; t++)
        Joystick.setButton(t, 0);
    }

    if (oldEna && oldEna != ena) {
      Joystick.setButton(14, HIGH);
      delay(BTN_DELAY);
      Joystick.setButton(14, LOW);
      SideEncoder.write(0);  // Reset pulses counter on ENA btn release
    }

    oldState = state;
  }
}