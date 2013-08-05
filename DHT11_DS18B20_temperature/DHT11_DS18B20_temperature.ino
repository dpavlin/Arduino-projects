// 
//   FILE:  dht11_test1.pde
// PURPOSE: DHT11 library test sketch for Arduino
//

//Celsius to Fahrenheit conversion
double Fahrenheit(double celsius)
{
	return 1.8 * celsius + 32;
}

// fast integer version with rounding
//int Celcius2Fahrenheit(int celcius)
//{
//  return (celsius * 18 + 5)/10 + 32;
//}


//Celsius to Kelvin conversion
double Kelvin(double celsius)
{
	return celsius + 273.15;
}

// dewPoint function NOAA
// reference: http://wahiduddin.net/calc/density_algorithms.htm 
double dewPoint(double celsius, double humidity)
{
	double RATIO = 373.15 / (273.15 + celsius);  // RATIO was originally named A0, possibly confusing in Arduino context
	double SUM = -7.90298 * (RATIO - 1);
	SUM += 5.02808 * log10(RATIO);
	SUM += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
	SUM += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
	SUM += log10(1013.246);
	double VP = pow(10, SUM - 3) * humidity;
	double T = log(VP/0.61078);   // temp var
	return (241.88 * T) / (17.558 - T);
}

// delta max = 0.6544 wrt dewPoint()
// 5x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity)
{
	double a = 17.271;
	double b = 237.7;
	double temp = (a * celsius) / (b + celsius) + log(humidity/100);
	double Td = (b * temp) / (a - temp);
	return Td;
}


#include <dht11.h>

dht11 DHT11;

#define DHT11PIN 2

#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 10

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

#define Backlight_Pin 9

void setup()
{
  Serial.begin(9600);
  Serial.println("DHT11 DS18B20 temperature");
  Serial.print("DHT11 LIBRARY VERSION: ");
  Serial.println(DHT11LIB_VERSION);
  Serial.println();

  // DS18B20
  sensors.begin();
  
  // Nokia 5110 display
  display.begin();

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(50);

  pinMode(Backlight_Pin, OUTPUT);

  pinMode(0, INPUT);
  Serial.print("Backlight ");
  Serial.println(analogRead(0));

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("Ready");
  display.display();
}

float dht11_temperature = 0;
float dht11_humidity = 0;
float ds18b20_temperature = 0;

#define TEMP_SIZE LCDWIDTH
float temp[TEMP_SIZE] = {0.0};
int temp_pos = 0; // position in circular buffer above

void loop()
{
  Serial.println("\n");

  int chk = DHT11.read(DHT11PIN);

  Serial.print("DHT11 Read sensor: ");
  switch (chk)
  {
    case DHTLIB_OK: 
		Serial.println("OK"); 
		break;
    case DHTLIB_ERROR_CHECKSUM: 
		Serial.println("Checksum error"); 
		break;
    case DHTLIB_ERROR_TIMEOUT: 
		Serial.println("Time out error"); 
		break;
    default: 
		Serial.println("Unknown error"); 
		break;
  }

  dht11_humidity = (float)DHT11.humidity;
  Serial.print("Humidity (%): ");
  Serial.println(dht11_humidity, 2);

  dht11_temperature = (float)DHT11.temperature;
  Serial.print("Temperature (oC): ");
  Serial.println(dht11_temperature, 2);

  Serial.print("Temperature (oF): ");
  Serial.println(Fahrenheit(DHT11.temperature), 2);

  Serial.print("Temperature (K): ");
  Serial.println(Kelvin(DHT11.temperature), 2);

  Serial.print("Dew Point (oC): ");
  Serial.println(dewPoint(DHT11.temperature, DHT11.humidity));

  Serial.print("Dew PointFast (oC): ");
  Serial.println(dewPointFast(DHT11.temperature, DHT11.humidity));


  Serial.print("DS18B20 Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.print("Temperature for the device 1 (index 0) is: ");
  ds18b20_temperature = sensors.getTempCByIndex(0);
  Serial.println(ds18b20_temperature);  

  temp[temp_pos] = ds18b20_temperature;

  display.clearDisplay();
  display.setCursor(0,0);
  display.print(dht11_temperature, 0);
  display.print("C ");
  display.print(dht11_humidity, 0);
  display.print("% ");
  display.print(ds18b20_temperature, 2);
  display.print("C");

  float min = temp[0], max = temp[0];
  
  for(int i = 0; i < TEMP_SIZE; i++) {
//    Serial.print(temp[i]);
//    Serial.print(" ");
    if (temp[i] < min && temp[i] > 0) min = temp[i];
    if (temp[i] > max) max = temp[i];
  }
  Serial.println();
  
  Serial.print("temperature range ");
  Serial.print(min);
  Serial.print("-");
  Serial.println(max);

  // draw right to left so most recent value is on the right
  for(int x = TEMP_SIZE - 1; x >= 0; x--) {
    int pos = ( x + temp_pos + 1 ) % TEMP_SIZE;
    if ( temp[pos] > 0 ) {
      int y = ( ( temp[pos] - min ) / ( max - min ) ) * ( LCDHEIGHT - 10 );
      display.drawLine(x, LCDHEIGHT - y, x, LCDHEIGHT, BLACK);
//      display.drawPixel(x,y + 10, BLACK);
//      Serial.print(temp[pos],2);
//      Serial.print(" ");
    }
  }
  Serial.println();

  // refresh LCD
  display.display();

  // pulse display backlight
  int backlight = 0;

  float old_temp = temp[(temp_pos + TEMP_SIZE - 1) % TEMP_SIZE];
  if ( ds18b20_temperature < old_temp ) {
    backlight = 32;
  } else if ( ds18b20_temperature > old_temp ) {
    backlight = 255;
  }
  analogWrite(Backlight_Pin, backlight);

  delay(2000);

  // move slot in circular bugger
  if ( ++temp_pos > TEMP_SIZE ) temp_pos = 0;

}
//
// END OF FILE
//
