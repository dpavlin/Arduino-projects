/*
    Over complicated desk LED dimmer
    2016-01-10 Dobrica Pavlinusic <dpavlin@rot13.org>
*/

const int led_pin = 13;
const int buzzer_pin = 4;
const int mosfet_pins[] = { 9, 10, 6 };
int mosfet_pwm[] = { 0, 0, 0 };

void setup() {
  Serial.begin(115200);

  pinMode(led_pin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);

  for(int i=0; i<sizeof(mosfet_pins); i++) pinMode(mosfet_pins[i], OUTPUT);

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

void loop() {
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
