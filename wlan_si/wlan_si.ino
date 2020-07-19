#include <Wire.h>
#include <Adafruit_MPL115A2.h>

Adafruit_MPL115A2 mpl115a2;

int led = 5; // LED on d5
int pir = 6; // PIR on d6

// hardware based on https://dev.wlan-si.net/wiki/Telemetry/sensgw
// original software https://github.com/SloMusti/sensgw

#include "RunningAverage.h"

RunningAverage pressureKPA_avg(100);
RunningAverage temperatureC_avg(100);

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Hello!");
  
  Serial.println("Getting barometric pressure ...");
  mpl115a2.begin();
  
  pinMode(led, OUTPUT);
  pinMode(pir, INPUT);
}

int count = 0;

void loop(void) 
{
  digitalWrite(led, HIGH);

  float pressureKPA = 0, temperatureC = 0;    

  mpl115a2.getPT(&pressureKPA,&temperatureC);
  pressureKPA_avg.addValue(pressureKPA);
  temperatureC_avg.addValue(temperatureC);

  if ( count++ % 10 == 0 ) {
  Serial.print("Pressure="); Serial.print(pressureKPA_avg.getAverage(), 4); Serial.print(" kPa ");
  Serial.print("Temp="); Serial.print(temperatureC_avg.getAverage(), 2); Serial.print(" C");

  Serial.print(" PIR="); Serial.print( digitalRead(pir) );

  Serial.print(" A0="); Serial.print( analogRead(0) );

  Serial.println();

  }

  digitalWrite(led, LOW);
  
  delay(100);
}
