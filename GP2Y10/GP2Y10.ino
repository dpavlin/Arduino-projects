/**
 * Example for using GP2Y1010AU0F Dust Sensor library
 * Created by Mickey Chan
 * library from https://github.com/mickey9801/GP2Y1010AU0F
 */

#include <GP2Y1010AU0F.h>

int measurePin = A0;   // Connect dust sensor analog measure pin to Arduino A0 pin
int ledPin     = 2;    // Connect dust sensor LED pin to Arduino pin 2

GP2Y1010AU0F dustSensor(ledPin, measurePin); // Construct dust sensor global object

void setup() {
  Serial.begin(115200);
  //Serial.println(F("GP2Y1010AU0F Dust Sensor Library Example"));

  dustSensor.begin();
}


#define samples 10
float dustDensity = 0;
int x;

void loop() {

  dustDensity = 0;
  for (x = 0; x < samples; x++) {

    dustDensity += dustSensor.read();
    delay( 1000 / samples );

  }
  
  Serial.print("Dust_Density=");
  Serial.println(dustDensity / samples);

}
