/*

  Connect Arduino ProMini 3.3V 8Mhz Atmega328 to Raspberry Pi
  
  RPI pin  Arduino
  RXD      TXD
  TXD      RXD
  GPIO6    RST
  GND      GND
  +5V      RAW

           2    433Mhz receive
//           3    433Mhz outdoor temperature sensor receiver # DISABLED
           8    DHT22
           10   433Mhz send
           11   DS18B20
           12   513Mhz send
           13   status LED
           
*/

#include <RCSwitch.h>

#include <OneWire.h>
#include <DallasTemperature.h>

RCSwitch mySwitch = RCSwitch();

// DS18B20 on pin 11
OneWire oneWire(11);
DallasTemperature sensors(&oneWire);


// 513Mhz light sockets
#define TX_PIN 12
#define LED_PIN 13

int int_0 = 300; // ms
int int_1 = 900; // ms
int wait  = 2000; // ms
int repeat = 5; // times

void send_513(char *code) {
  Serial.print("send 513Mhz ");
  Serial.println(code);
  
  // we have to send same signal at least two times
  for(int r = 0; r < repeat; r++ ) {

    digitalWrite(LED_PIN, HIGH);

    for(int i = 0; i < strlen(code); i++) {
      int i1 = int_0;
      int i2 = int_1;
      if (code[i] == '1' ) {
        i1 = int_1;
        i2 = int_0;
      }
      digitalWrite(TX_PIN, HIGH);
      delayMicroseconds(i1);
      digitalWrite(TX_PIN, LOW);
      delayMicroseconds(i2);
    }

    delayMicroseconds(wait); // guess
  }

  digitalWrite(LED_PIN, LOW);
}


// DHT22
#include "DHT.h"
DHT dht;

// setup

void help() {
  Serial.print("# press buttons on remote or send AB where A = socket (0..9), B = state (0 = off, 1 = on)\nB00...00 (24 digits) to send binary\n");
}

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(0);  // Receiver on inerrupt 0 => that is pin #2  
  mySwitch.enableTransmit(10); // with sender wired in receiving doesn't work, pin #10

  // DS18B20
  sensors.begin();

  // DHT22
  dht.setup(8);

}

int serial_pos = 0;
char serial_data[2]; // socket (0-9), state (0-1)
char binary_data[24];

void loop() {
  if (mySwitch.available()) {
    Serial.print(mySwitch.getReceivedBitlength());
    Serial.print(" bits ");
    Serial.println(mySwitch.getReceivedValue(), BIN);
    mySwitch.resetAvailable();
  }
  if (Serial.available() > 0) {
     char input = Serial.read();

     if (input == '?' || input == 'h') {
       help();
     } else

     if ( input == 'T' ) {
       Serial.print("DS18B20 temperature = ");
       sensors.requestTemperatures();
       Serial.println( sensors.getTempCByIndex(0) );       
     } else

     if ( input == 'B' ) {
       Serial.readBytesUntil('\n', binary_data, sizeof(binary_data));
       Serial.print("# send B");
       Serial.println( binary_data );
       mySwitch.send( binary_data );
     } else

    // light sockets at 513 Mhz
     if (input == 'a') {
       send_513("1000100110110000000000010");
     } else
     if (input == 'b') {
       send_513("1011001001011111000000010");
     } else

     // DHT22
     if (input == 'd') {
       Serial.print("temperature=");
       Serial.print(dht.getTemperature());
       Serial.print(" humidity=");
       Serial.print(dht.getHumidity());
       Serial.print(" status=");
       Serial.println(dht.getStatusString());
     }

     if ( input >= 0x30 && input <= 0x39 ) {
       input = input - 0x30; // ASCII to number
       serial_data[serial_pos++] = input;
     } else {     
       Serial.print("# ignore: ");
       Serial.println(input, HEX);
     }
     
     if ( serial_pos == 2 ) {
       Serial.print("switch=");
       Serial.print(serial_data[0], DEC);
       Serial.print(" state=");
       Serial.println(serial_data[1] ? "on" : "off");

       byte on = serial_data[1];

	// switches, 433 Mhz set of 3
	switch ( serial_data[0] ) {
         case 1:
           on	? mySwitch.send("110101011101010000001100")
		: mySwitch.send("110101011101010000000011");
	   break;
         case 2:
           on	? mySwitch.send("110101010111010000001100")
           	: mySwitch.send("110101010111010000000011");
           break;
         case 3:
           on	? mySwitch.send("110101010101110000001100")
           	: mySwitch.send("110101010101110000000011");
           break;
         case 4:
           on	? mySwitch.send("001111110000000011000000")
           	: mySwitch.send("001111110000000000000000");
           break;
         default:
           Serial.print("# invalid switch number ");
           Serial.println(serial_data[0], DEC);
	}

	// reset for later
	serial_pos = 0;
     }
  }
}
