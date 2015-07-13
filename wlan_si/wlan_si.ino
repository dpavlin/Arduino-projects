#include <Wire.h>
#include <Adafruit_MPL115A2.h>

Adafruit_MPL115A2 mpl115a2;

int led = 5; // LED on d5
int pir = 6; // PIR on d6

// hardware based on https://dev.wlan-si.net/wiki/Telemetry/sensgw
// original software https://github.com/SloMusti/sensgw

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Hello!");
  
  Serial.println("Getting barometric pressure ...");
  mpl115a2.begin();
  
  pinMode(led, OUTPUT);
  pinMode(pir, INPUT);
}

void loop(void) 
{
  digitalWrite(led, HIGH);

  float pressureKPA = 0, temperatureC = 0;    

  mpl115a2.getPT(&pressureKPA,&temperatureC);
  Serial.print("Pressure="); Serial.print(pressureKPA, 4); Serial.print(" kPa ");
  Serial.print("Temp="); Serial.print(temperatureC, 1); Serial.print(" C");

  Serial.print(" PIR="); Serial.print( digitalRead(pir) );

  Serial.print(" A0="); Serial.print( analogRead(0) );

  Serial.println();

  digitalWrite(led, LOW);
  
  delay(1000);
}
