#include <Arduino.h>

void btn_init()
{
  int i;
  // buttons
  for(i = 0; i < 6; i++)
    pinMode(i, INPUT);
  // switches
  for(i = 16; i < 20; i++)
    pinMode(i, INPUT);
  // leds
  for(i = 8; i < 16; i++)
    pinMode(i, OUTPUT);
}

uint8_t btn[6];
uint8_t btn_toggle[6] = { 0,0,0,0,0,0 };

int btn_read(char *line)
{
  int ret = 0;
  static uint8_t shiftdata = 0;
  uint8_t led = B10001 << (shiftdata++ & 3);

  Serial.print("btn_toggle=");
  for(int i=0; i<=6; i++) {
    btn[i] = digitalRead(i);
    if ( i == 0 ) btn[i] = !btn[i];
    if ( btn[i] ) {
      digitalWrite(8+i, 1);
      if ( btn_toggle[i] == 0 ) {
        btn_toggle[i]++; // press
      }
    } else {
      digitalWrite(8+i, 0);
      if ( btn_toggle[i] == 1 ) {
        btn_toggle[i]++; // release
      }
    }
    if (btn_toggle[i] == 2) {
      ret++;
      //digitalWrite(8+i, 1);
    }
    Serial.print(btn_toggle[i]);
  }
  Serial.println();
  
  sprintf(line, "BTN:%c%c%c%c%c%c%c SW:%c%c%c%c LED:%c%c%c%c%c%c%c%c\n",
    digitalRead(0) ? '_' : '0',
    digitalRead(1) ? '1' : '_',
    digitalRead(2) ? '2' : '_',
    digitalRead(3) ? '3' : '_',
    digitalRead(4) ? '4' : '_',
    digitalRead(5) ? '5' : '_',
    digitalRead(6) ? '6' : '_',
    digitalRead(16) ? '1' : '_',
    digitalRead(17) ? '2' : '_',
    digitalRead(18) ? '3' : '_',
    digitalRead(19) ? '4' : '_',
    led & B10000000 ? '7' : '_',
    led & B01000000 ? '6' : '_',
    led & B00100000 ? '5' : '_',
    led & B00010000 ? '4' : '_',
    led & B00001000 ? '3' : '_',
    led & B00000100 ? '2' : '_',
    led & B00000010 ? '1' : '_',
    led & B00000001 ? '0' : '_'
  );
  //for(int i = 0; i < 8; i++)
  //  digitalWrite(8+i, led & (1<<i) ? 1 : 0);
  return ret;
}
