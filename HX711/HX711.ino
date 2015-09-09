#define FLOAT_AVG 0

#include "HX711.h"

// HX711.DOUT	- pin #A2
// HX711.PD_SCK	- pin #A1

HX711 scale(A2, A1);		// parameter "gain" is ommited; the default value 128 is used by the library

#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // rs enable d0-d3

void setup() {
  lcd.begin(16, 2);
  lcd.print("HX711");

  Serial.begin(115200);
  Serial.println("HX711");

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());			// print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));  	// print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);	// print the average of 5 readings from the ADC minus tare weight (not set) divided 
						// by the SCALE parameter (not set yet)  
  scale.set_scale(465.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();				        // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided 
						// by the SCALE parameter set with set_scale

  Serial.println("Readings:");
}

#ifdef FLOAT_AVG
#define g_bucket 20
#define g_size g_bucket*3
float g_history[g_size];
int g_pos = 0;
#endif

void loop() {

  lcd.setCursor(0,1);
  uint32_t adc = scale.read();
  lcd.print(adc);
  lcd.print(" ");

  Serial.print(adc);
  Serial.print("\t");

  float g = scale.get_units();
  Serial.print(g);
 
  lcd.setCursor(0,0); // col,row
  lcd.print(g);
  lcd.print(" ");

#if FLOAT_AVG
  g_history[g_pos++] = g; // insert into circular buffer
  g_pos %= g_size;

  float sum = 0;
  for(int i=1; i<g_size; i++) {
    int pos = ( g_pos + i ) % g_size;
    sum += g_history[pos];
    if ( i % g_bucket == 0) {
	float avg = sum / i;
	Serial.print("\t");
	Serial.print(avg, 2);
        lcd.print(avg);
        lcd.print(" ");
    }
  }
#endif
 
  int key = analogRead(0);
  Serial.print("\t");
  Serial.print(key);

  lcd.setCursor(12,1);
  lcd.print(key);
  lcd.print("   ");

  if ( key > 1000 ) {
	// nothing pressed
  } else if ( key > 800 ) {
	lcd.print(" tare ");
        scale.tare();
  }


//  scale.power_down();			        // put the ADC in sleep mode
//  delay(100);
//  scale.power_up();
  Serial.println();
}
