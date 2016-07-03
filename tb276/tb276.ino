/*
 Double Fade


 This example shows how to fade an LEDs on pin 9 and 10
 using the analogWrite() function.

 This example code is in the public domain.
 */

int led1 = 9;           // the pin that the LED is attached to
int led2 = 10;
int led3 = 11;
int led4 = 12;
int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by
int count = 0;

// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output:
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  analogWriteResolution(12);
  digitalWrite(led1, HIGH);
  digitalWrite(led2, LOW);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, LOW);

  Serial.begin(115200);
  Serial.println("setup");
}

// the loop routine runs over and over again forever:
void loop() {
  static int mode = 0;
  // set the brightness of pin 9:
  mode++;
  mode &= 16383;
  if((mode & 8192) == 0)
  {
    analogWrite(led1, brightness);
    analogWrite(led2, 4095-brightness);
    digitalWrite(led3, (mode & 256) == 0 ? LOW  : HIGH);
    digitalWrite(led4, (mode & 256) == 0 ? HIGH : LOW);
  }
  else
  {
    digitalWrite(led1, (mode & 256) == 0 ? LOW  : HIGH);
    digitalWrite(led2, (mode & 256) == 0 ? HIGH : LOW);
    analogWrite(led3, brightness);
    analogWrite(led4, 4095-brightness);
  }

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;
  if (brightness % 100 == 0 ) {
    Serial.print( int(brightness / 100) );
    Serial.print(" ");
  }

  // reverse the direction of the fading at the ends of the fade:
  if (brightness == 0 || brightness == 4095) {
    fadeAmount = -fadeAmount ;
    Serial.println();
    Serial.print(micros());
    Serial.print(" ");
    Serial.print(count++);
    Serial.print(" ");
  }
  // wait for 30 milliseconds to see the dimming effect
  delay(1);
}
