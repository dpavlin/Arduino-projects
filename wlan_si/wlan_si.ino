#include <Wire.h>
#include <Adafruit_MPL115A2.h>

Adafruit_MPL115A2 mpl115a2;

int led = 5; // LED on d5

// hardware based on https://dev.wlan-si.net/wiki/Telemetry/sensgw
// original software https://github.com/SloMusti/sensgw

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Hello!");
  
  Serial.println("Getting barometric pressure ...");
  mpl115a2.begin();
  
  pinMode(led, OUTPUT);
}

void loop(void) 
{
  digitalWrite(led, HIGH);

  float pressureKPA = 0, temperatureC = 0;    

  mpl115a2.getPT(&pressureKPA,&temperatureC);
  Serial.print("Pressure: "); Serial.print(pressureKPA, 4); Serial.println(" kPa");
  Serial.print("Temp: "); Serial.print(temperatureC, 1); Serial.println(" *C");

/*  
  pressureKPA = mpl115a2.getPressure();  
  Serial.print("Pressure (kPa): "); Serial.print(pressureKPA, 4); Serial.println(" kPa");

  temperatureC = mpl115a2.getTemperature();  
  Serial.print("Temp (*C): "); Serial.print(temperatureC, 1); Serial.println(" *C");
*/

  digitalWrite(led, LOW);
  
  delay(1000);
}
