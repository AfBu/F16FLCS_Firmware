#include <Joystick.h>

#define LATCH_PIN 8
#define DATA_PIN 9
#define CLOCK_PIN 7

#define HID_ID 0x16
#define BUTTONS_COUNT 14
#define HATS_COUNT 2
#define SWITCH_PIN 4

//#define DEBUG 1

Joystick_ Joystick = Joystick_(
    HID_ID,                 // HID
    JOYSTICK_TYPE_JOYSTICK, // TYPE
    BUTTONS_COUNT + 1,      // BUTTONS
    HATS_COUNT,             // HATS
    true,                   // X-AXIS
    true,                   // Y-AXIS
    false,                  // Z-AXIS
    false,                  // RX-AXIS
    false,                  // RY-AXIS
    false,                  // RZ-AXIS
    false,                  // RUDDER
    false,                  // THROTTLE
    false,                  // ACCELERATOR
    false,                  // BRAKE
    false                   // STEERING
    );

byte chip1, chip2, chip3, _chip1, _chip2, _chip3, i;
bool buttons[BUTTONS_COUNT];
bool switchState, lastSwitchState;
char binText[9];
int hat0, hat1;

void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  Serial.println("ThrustMaster F-16 FLCS Debug Firmware");
  #endif

  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  Joystick.begin(false);

  binText[8] = 0;
}

void printBin(byte h)
{
  #ifdef DEBUG
  for (i = 0; i < 8; i++) {
    binText[i] = ((h >> i) & 0x01 ? '1' : '0');
  }
  Serial.print(binText);
  #endif
}

void loop() {
  digitalWrite(LATCH_PIN, 1);
  delayMicroseconds(20);
  digitalWrite(LATCH_PIN,0);

  chip1 = shiftIn(DATA_PIN, CLOCK_PIN, LSBFIRST) xor 0xFF;
  chip2 = shiftIn(DATA_PIN, CLOCK_PIN, LSBFIRST) xor 0xFF;
  chip3 = shiftIn(DATA_PIN, CLOCK_PIN, LSBFIRST) xor 0xFF;

  switchState = (digitalRead(SWITCH_PIN) == HIGH);
  if (switchState != lastSwitchState) {
    lastSwitchState = switchState;
    if (switchState) {
      Joystick.pressButton(14);
    } else {
      Joystick.releaseButton(14);
    }
  }

  if (chip1 != _chip1 || chip2 != _chip2 || chip3 != _chip3) {
    _chip1 = chip1;
    _chip2 = chip2;
    _chip3 = chip3;

    // hat 0
    hat0 = -1;
    if (chip2 & 0x02) { // up
      hat0 = 0;
      if (chip2 & 0x04) { // up-left
        hat0 = 315;
      } else if (chip2 & 0x01) { // up-right
        hat0 = 45;
      }
    } else if (chip1 & 0x80) { // down
      hat0 = 180;
      if (chip2 & 0x04) { // down-left
        hat0 = 225;
      } else if (chip2 & 0x01) { // down-right
        hat0 = 135;
      }
    } else if (chip2 & 0x04) { // left
      hat0 = 270;
    } else if (chip2 & 0x01) { // right
      hat0 = 90;
    }
    Joystick.setHatSwitch(0, hat0);

    // hat 1
    hat1 = -1;
    if (chip3 & 0x02) { // up
      hat1 = 0;
      if (chip3 & 0x04) { // up-left
        hat1 = 315;
      } else if (chip3 & 0x01) { // up-right
        hat1 = 45;
      }
    } else if (chip2 & 0x80) { // down
      hat1 = 180;
      if (chip3 & 0x04) { // down-left
        hat1 = 225;
      } else if (chip3 & 0x01) { // down-right
        hat1 = 135;
      }
    } else if (chip3 & 0x04) { // left
      hat1 = 270;
    } else if (chip3 & 0x01) { // right
      hat1 = 90;
    }
    Joystick.setHatSwitch(1, hat1);

    // buttons
    buttons[0] = (chip1 & 0x01); // fire 1
    buttons[1] = (chip1 & 0x02); // fire 2
    buttons[2] = (chip1 & 0x10); // top button
    buttons[3] = (chip1 & 0x04); // side button
    buttons[4] = (chip3 & 0x80); // bottom button
    buttons[5] = (chip1 & 0x08); // paddle button

    // hats
    buttons[6] = (chip3 & 0x20); // hat 3 up
    buttons[7] = (chip3 & 0x08); // hat 3 dn
    buttons[8] = (chip3 & 0x40); // hat 3 lf
    buttons[9] = (chip3 & 0x10); // hat 3 rt
    
    buttons[10] = (chip2 & 0x08); // hat 3 up
    buttons[11] = (chip2 & 0x20); // hat 3 dn
    buttons[12] = (chip2 & 0x10); // hat 3 lf
    buttons[13] = (chip2 & 0x40); // hat 3 rt

    for (i = 0; i < BUTTONS_COUNT; i++) {
      if (buttons[i]) {
        Joystick.pressButton(i);
      } else {
        Joystick.releaseButton(i);
      }
    }
}

  #ifdef DEBUG
  Serial.println("");
  Serial.print("CHIP 1: ");
  Serial.println(chip1, HEX);
  Serial.print("CHIP 2: ");
  Serial.println(chip2, HEX);
  Serial.print("CHIP 3: ");
  Serial.println(chip3, HEX);
  delay(100);
  #endif

  Joystick.setXAxis(analogRead(A0));
  Joystick.setYAxis(1023-analogRead(A1));
  Joystick.sendState();
}
