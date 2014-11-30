/*

  Connect Arduino ProMini 3.3V 8Mhz Atmega328 to Raspberry Pi
  
  RPI pin  Arduino
  RXD      TXD
  TXD      RXD
  GPIO6    RST
  GND      GND
  +5V      RAW

           2    433Mhz receive
           10   433Mhz send
           11   DS18B20

*/

#include <RCSwitch.h>

#include <OneWire.h>
#include <DallasTemperature.h>

RCSwitch mySwitch = RCSwitch();

// DS18B20 on pin 11
OneWire oneWire(11);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(0);  // Receiver on inerrupt 0 => that is pin #2  
  mySwitch.enableTransmit(10); // with sender wired in receiving doesn't work, pin #10

  // DS18B20
  sensors.begin();

  Serial.print("press buttons on remote or send AB where A = socket (0..9), B = state (0 = off, 1 = on)\nB00...00 (24 digits) to send binary\n");
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
     if ( input == 'T' ) {
       Serial.print("DS18B20 temperature = ");
       sensors.requestTemperatures();
       Serial.println( sensors.getTempCByIndex(0) );       
     } else
     if ( input == 'B' ) {
       Serial.readBytesUntil('\n', binary_data, sizeof(binary_data));
       Serial.print("send B");
       Serial.println( binary_data );
       mySwitch.send( binary_data );
     } else
     if ( input >= 0x30 && input <= 0x39 ) {
       input = input - 0x30; // ASCII to number
       serial_data[serial_pos++] = input;
     } else {     
       Serial.print("ignore: ");
       Serial.println(input, HEX);
     }
     
     if ( serial_pos == 2 ) {
       Serial.print("socket: ");
       Serial.print(serial_data[0], DEC);
       Serial.print(" state: ");
       Serial.println(serial_data[1] ? "on" : "off");
       serial_pos = 0;
       if ( serial_data[1] ) { // on
         switch ( serial_data[0] ) {
         case 1:
           mySwitch.send("110101011101010000001100");
           break;
         case 2:
           mySwitch.send("110101010111010000001100");
           break;
         case 3:
           mySwitch.send("110101010101110000001100");
           break;
         default:
           Serial.print("invalid switch on number ");
           Serial.println(serial_data[0], DEC);
         }
       } else { // off
         switch ( serial_data[0] ) {
         case 1:
           mySwitch.send("110101011101010000000011");
           break;
         case 2:
           mySwitch.send("110101010111010000000011");
           break;
         case 3:
           mySwitch.send("110101010101110000000011");
           break;
         default:
           Serial.print("invalid switch off number ");
           Serial.println(serial_data[0], DEC);
         }
       }
     }
  }
}
