/*

scrolling led on buttons

 */

extern "C" {
#include <dev/io.h> // needed for INB
}

#define NR_LEDS 8
int leds[NR_LEDS] = {8,9,10,11,12,13,14,15};           // the pin that the LED is attached to
	// I don't really understand where this numbers are comming from yet :-(
int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by
int count = 0;
int pos = 0;
int old_key = -1;
int key_micros = 0;

// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output:
  for(int i=0; i<NR_LEDS; i++) {
	pinMode(leds[i], OUTPUT);
	digitalWrite(leds[i], LOW);
  }

  analogWriteResolution(12);

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
    analogWrite(leds[pos], brightness);
  }
  else
  {
    digitalWrite(leds[pos], (mode & 256) == 0 ? LOW  : HIGH);
  }

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness == 0 || brightness == 4095) {
    fadeAmount = -fadeAmount ;
    Serial.print(micros());
    Serial.print(" ");
    Serial.println(count++);
  }


  int old_pos = pos;
  int key;
  INB(key, IO_PUSHBTN);
  if ( key != old_key && micros() - key_micros > 300) {
	old_key = key;
	if (key & BTN_RIGHT && pos >= 0) pos--;
	if (key & BTN_LEFT  && pos < (NR_LEDS-1)) pos++;
	if ( pos != old_pos ) {
		Serial.print("key ");
		Serial.print(key);
		Serial.print(" pos=");
		Serial.print(pos);
		Serial.print(" micros diff=");
		Serial.println(micros()-key_micros);
		digitalWrite(leds[old_pos], LOW);
		digitalWrite(leds[pos], HIGH);
	}
	key_micros = micros();
  }


  // wait for 30 milliseconds to see the dimming effect
  delay(1);
}
