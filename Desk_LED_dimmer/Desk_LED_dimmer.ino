/*
    Over complicated desk LED dimmer
    2016-01-10 Dobrica Pavlinusic <dpavlin@rot13.org>
*/

const int led_pin = 13;
const int buzzer_pin = 4;
/* ENC28J60 pins
SCK D13
SO  D12
SI  D11
CS  D10
#define ENC28J60_CONTROL_CS                     10 // remap?
*/
const int mosfet_pins[] = { 9, 10, 6 }; // PWM pins: 3,5,6,9,10,11
const int ldr_pin = A3; // LDR +5 -- A3 -[10K]- GND
const int pir_pin = A2;

#define TOUCHPAD 1 // set this to 0 if debugging without touchpad
#define PIR_TIMEOUT 10 // s

#if TOUCHPAD
#include <ps2.h>
PS2 mouse(8, 7); // PS2 synaptics clock, data
#endif

int mosfet_pwm[] = { 255, 255, 255 }; // initial and current state of mosfet pwm

void setup() {
  Serial.begin(115200);
  Serial.println("setup");

  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, HIGH); // signal reset

  pinMode(buzzer_pin, OUTPUT);

  for(int i=0; i<=2; i++) {
    pinMode(mosfet_pins[i], OUTPUT);
    analogWrite(mosfet_pins[i], mosfet_pwm[i]);
  }

  pinMode(ldr_pin, INPUT);
  pinMode(pir_pin, INPUT);

#if TOUCHPAD
  Serial.println("Synaptics touchpad init");

  mouse.write(0xff);  // reset
  mouse.read();  // ack byte
  mouse.read();  // blank */

  mouse.read();  // blank */
  mouse.write(0xf0);  // remote mode -- send motion data only on $EB (read data) command
  mouse.read();  // ack
  delayMicroseconds(100);
  mouse.write(0xe8); // set resolution
  mouse.read();  // ack byte
  mouse.write(0x03); // rr  ( rr * 64  +  ss * 16  +  tt * 4  +  uu  == modebyte )
  mouse.read();  // ack byte
  mouse.write(0xe8);
  mouse.read();  // ack byte
  mouse.write(0x00); // ss
  mouse.read();  // ack byte
  mouse.write(0xe8);
  mouse.read();  // ack byte
  mouse.write(0x01); // tt
  mouse.read();  // ack byte
  mouse.write(0xe8);
  mouse.read();  // ack byte
  mouse.write(0x00); // uu
  mouse.read();  // ack byte
  mouse.write(0xf3); // set samplerate 20 (stores mode) 
  mouse.read();  // ack byte
  mouse.write(0x14);
  mouse.read();  // ack byte
  delayMicroseconds(100); 

#endif

  Serial.println("Commands: b - beep, qwe/asd/zxc - MOSFETs");

  digitalWrite(led_pin, LOW);

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

int last_ldr = 0;
// number of LDR reading to average
#define LDR_SIZE 100
static int ldr_sum = 0;
static int ldr_count = 0;

int last_pir = 0;
long int pir_millis = millis();

void loop() {

#if TOUCHPAD
  byte mstat1;
  byte mstat2;
  byte mxy;
  byte mx;                    //x coordinate
  byte my;                    //y coordinate
  byte mz;                    //z pressure value

  unsigned int cx,cy;

  mouse.write(0xeb); // read data
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
#endif


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

  int ldr = analogRead(ldr_pin);
  ldr_sum += ldr >> 2;
  ldr_count++;
  if ( ldr_count > LDR_SIZE ) {
    ldr = ldr_sum / ldr_count;
    ldr_count = 0;
    ldr_sum = 0;

    if ( abs(ldr - last_ldr) > 5 ) {  
      Serial.print("LDR = ");
      Serial.println(ldr);
    }
    last_ldr = ldr;
  }


  int pir = digitalRead(pir_pin);
  if ( pir != last_pir) {
    last_pir = pir;
    Serial.print("PIR = ");
    Serial.println(pir);
  }

  long int ms = millis();
  if ( pir == 0 ) {
    if (pir_millis > 0 && ms - pir_millis > PIR_TIMEOUT * 1000 ) {
      Serial.println("PIR timeout, fade-out");
      pir_millis = -1; // mark that we are in timeout
      for(int i=0; i<=2; i++) {
        analogWrite(mosfet_pins[i], 0);
      }
    }
  } else {
    if (pir_millis < 0) {
      Serial.println("PIR fade-in after timeout");
      for(int i=0; i<=2; i++) {
        analogWrite(mosfet_pins[i], mosfet_pwm[i]);
      }
    }
    pir_millis = ms;
  }

}
