/**
The MIT License (MIT)

Copyright (c) 2015 richardhmm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
/**
   connect the sensor as follows :
          Pin 2 of dust sensor PM1.0      -> Digital 3
	  Pin 3 of dust sensor          -> +5V 
	  Pin 4 of dust sensor PM2.5    -> Digital 4
	  Pin 5 of dust sensor          -> Ground
  Datasheet: http://www.samyoungsnc.com/products/3-1%20Specification%20DSM501.pdf
*/
#include<DSM501.h>

#define DSM501_PM10 3
#define DSM501_PM25 4

DSM501 dsm501(DSM501_PM10, DSM501_PM25);

void setup()
{
  Serial.begin(9600);	//for output information

  // Initialize DSM501
  dsm501.begin(MIN_WIN_SPAN);

  // wait 60s for DSM501 to warm up
  for (int i = 1; i <= 60; i++)
  {
    delay(1000); // 1s
    Serial.print(i);
    Serial.println(" s (wait 60s for DSM501 to warm up)");
  }
}

void loop()
{
  // call dsm501 to handle updates.
  dsm501.update();
  
  // get PM density of particles over 1.0 μm
  Serial.print("PM10: ");
  Serial.print(dsm501.getParticalWeight(0));
  Serial.println(" ug/m3");
  
  // get PM density of particles over 2.5 μm
  Serial.print("PM25: ");
  Serial.print(dsm501.getParticalWeight(1));
  Serial.println(" ug/m3");
  
  Serial.print("AQI: ");
  Serial.println(dsm501.getAQI());
  
  // get PM2.5 density of particles between 1.0~2.5 μm
  Serial.print("PM2.5: ");
  Serial.print(dsm501.getPM25());
  Serial.println(" ug/m3");
}







