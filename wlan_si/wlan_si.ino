#include <Wire.h>
#include <Adafruit_MPL115A2.h>

Adafruit_MPL115A2 mpl115a2;

int led = 5; // LED on d5
int pir = 6; // PIR on d6

// hardware based on https://dev.wlan-si.net/wiki/Telemetry/sensgw
// original software https://github.com/SloMusti/sensgw

float runningAverage(float M) {
  #define LM_SIZE 100
  static float LM[LM_SIZE];      // LastMeasurements
  static byte index = 0;
  static float sum = 0;
  static byte count = 0;

  // keep sum updated to improve speed.
  sum -= LM[index];
  LM[index] = M;
  sum += LM[index];
  index++;
  index = index % LM_SIZE;
  if (count < LM_SIZE) count++;

  return sum / count;
}

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

  if ( count++ % 10 == 0 ) {
  Serial.print("Pressure="); Serial.print(runningAverage(pressureKPA), 4); Serial.print(" kPa ");
  Serial.print("Temp="); Serial.print(temperatureC, 1); Serial.print(" C");

  Serial.print(" PIR="); Serial.print( digitalRead(pir) );

  Serial.print(" A0="); Serial.print( analogRead(0) );

  Serial.println();

  } else {
	runningAverage(pressureKPA);
  }

  digitalWrite(led, LOW);
  
  delay(100);
}
