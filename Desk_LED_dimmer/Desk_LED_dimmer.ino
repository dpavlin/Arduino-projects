/*
    Over complicated desk LED dimmer
    2016-01-10 Dobrica Pavlinusic <dpavlin@rot13.org>
*/

const int led_pin = 13;
const int buzzer_pin = 4;
const int mosfet_pins[] = { 9, 10, 6 };
int mosfet_pwm[] = { 0, 0, 0 };

#include <ps2.h>
PS2 mouse(8, 7); // PS2 synaptics clock, data

void setup() {
  Serial.begin(115200);

  pinMode(led_pin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);

  for(int i=0; i<sizeof(mosfet_pins); i++) pinMode(mosfet_pins[i], OUTPUT);


  mouse.write(0xff);  // reset
  mouse.read();  // ack byte
  mouse.read();  // blank */
  mouse.read();  // blank */
  mouse.write(0xf0);  // remote mode
  mouse.read();  // ack
  delayMicroseconds(100);
  mouse.write(0xe8);
  mouse.read();  // ack byte
  mouse.write(0x03); // x1  ( x1 * 64  +  x2 * 16  +  x3 * 4  +  x4   == modebyte )
  mouse.read();  // ack byte
  mouse.write(0xe8);
  mouse.read();  // ack byte
  mouse.write(0x00); // x2
  mouse.read();  // ack byte
  mouse.write(0xe8);
  mouse.read();  // ack byte
  mouse.write(0x01); // x3
  mouse.read();  // ack byte
  mouse.write(0xe8);
  mouse.read();  // ack byte
  mouse.write(0x00); // x4
  mouse.read();  // ack byte
  mouse.write(0xf3); // set samplerate 20 (stores mode) 
  mouse.read();  // ack byte
  mouse.write(0x14);
  mouse.read();  // ack byte
  delayMicroseconds(100); 


  Serial.println("Commands: b - beep, qwe/asd/zxc - MOSFETs");

}

void mosfet(int nr, int value) {
  int pwm = mosfet_pwm[nr];
  if ( pwm == value ) {
    Serial.println("ignored");
    return;
  }
  Serial.print("MOSFET ");
  Serial.print(nr);
  Serial.print(" = ");
  Serial.println(value);
  int sleep = 1000 / abs(pwm - value);
  int step = pwm < value ? 1 : -1;
  for(int i=pwm; i != value; i += step) {
    analogWrite(mosfet_pins[nr], i);
    Serial.println(i);
    delay(sleep);
  }
  analogWrite(mosfet_pins[nr], value);
  mosfet_pwm[nr] = value;
}


unsigned int last_cx = 0;
unsigned int last_cy = 0;         


void loop() {

  byte mstat1;
  byte mstat2;
  byte mxy;
  byte mx;                    //x coordinate
  byte my;                    //y coordinate
  byte mz;                    //z pressure value

  unsigned int cx,cy;

  mouse.write(0xeb);
  mouse.read();
  
  mstat1 = mouse.read();
  mxy = mouse.read();
  mz = mouse.read();
  mstat2 = mouse.read();
  mx = mouse.read();
  my = mouse.read();


  // collect the bits for x and y
  cx = (((mstat2 & 0x10) << 8) | ((mxy & 0x0F) << 8) | mx ); // 1100-5800
  cy = (((mstat2 & 0x20) << 7) | ((mxy & 0xF0) << 4) | my ); // 800-5000

  if ( last_cx != cx || last_cy != cy ) {
    last_cx = cx;
    last_cy = cy;
    Serial.print("X=");
    Serial.print(cx, DEC);
    Serial.print("\tY=");
    Serial.print(cy, DEC);
    Serial.print("\tZ=");
    Serial.print(mz, DEC);
    if ( cx > 1100 && cy > 800 ) {
      int nr  = ( cx - 1100 ) / (( 5800 - 1100 ) / 3);
      int pwm = ( cy - 800  ) / (( 5000 - 800  ) / 255);
      pwm -= 1; // allow off
      Serial.print("\tmosfet = ");
      Serial.print(nr);
      Serial.print("\tpwm = ");
      Serial.print(pwm);
      if ( nr >= 0 && nr <= 2 && pwm >= 0 && pwm <= 255 ) {
        analogWrite(mosfet_pins[nr], pwm);
        Serial.print("\tOK");
      } else {
        Serial.print("\tIGNORED");
      }
    }
    Serial.println();
  }
  

  if (Serial.available()) {
    digitalWrite(led_pin, HIGH);
    int in = Serial.read();
    switch (in) {
      case 'b':
        tone(buzzer_pin, 1800, 100);
        delay(200);
        tone(buzzer_pin, 2200, 100);
        delay(500);
        break;
      case 'q': mosfet(0, 255); break;
      case 'a': mosfet(0, 127); break;
      case 'z': mosfet(0, 0); break;
      case 'w': mosfet(1, 255); break;
      case 's': mosfet(1, 127); break;
      case 'x': mosfet(1, 0); break;
      case 'e': mosfet(2, 255); break;
      case 'd': mosfet(2, 127); break;
      case 'c': mosfet(2, 0); break;

      /*
              m1 = (m1 + 10) % 255;
              analogWrite(mosfet1_pin, m1);
              Serial.print("MOSFET 1 = ");
              Serial.println(m1);
              break;
            case 'a':
              m1 = (m1 - 10) % 255;
              analogWrite(mosfet1_pin, m1);
              Serial.print("MOSFET 1 = ");
              Serial.println(m1);
              break;
      */
      default:
        Serial.print("unknown command ");
        Serial.println(in);
    }
    digitalWrite(led_pin, LOW);
  }
}
