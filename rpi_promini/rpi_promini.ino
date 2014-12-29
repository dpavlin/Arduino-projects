/*

  Connect Arduino ProMini 3.3V 8Mhz Atmega328 to Raspberry Pi
  
  RPI pin  Arduino
  RXD      TXD
  TXD      RXD
  GPIO6    RST
  GND      GND
  +5V      RAW

           2    433Mhz receive
           3    433Mhz outdoor temperature sensor receiver
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


// outdoor 433 MHz temperature sensor

// Unknown 433Mhz weather sensor decoder. Untested in the real world.
// http://arduino.cc/forum/index.php/topic,142871.msg1106336.html#msg1106336

// __           ___       ___    ___
//   |         |  |      |  |   |  |
//   |_________|  |______|  |___|  |
//
//   |  Sync      |    1    |  0   |
//   |  9780us    | 4420us  | 2410us
//      9810        4410      2490

// Defines
#define DataBits0 4                                       // Number of data0 bits to expect
#define DataBits1 32                                      // Number of data1 bits to expect
#define allDataBits 36                                    // Number of data sum 0+1 bits to expect
// isrFlags bit numbers
#define F_HAVE_DATA 1                                     // 0=Nothing in read buffer, 1=Data in read buffer
#define F_GOOD_DATA 2                                     // 0=Unverified data, 1=Verified (2 consecutive matching reads)
#define F_CARRY_BIT 3                                     // Bit used to carry over bit shift from one long to the other
#define F_STATE 7                                         // 0=Sync mode, 1=Data mode

// uno: pin 2 = int.0 or pin 3 = int.1 see
// http://arduino.cc/en/Reference/attachInterrupt
#define ISR_PIN 3
#define ISR_INT 1

// Constants
const unsigned long sync_MIN = 9000;                      // Minimum Sync time in micro seconds
const unsigned long sync_MAX = 10000;

const unsigned long bit1_MIN = 4000;
const unsigned long bit1_MAX = 5000;

const unsigned long bit0_MIN = 2000;
const unsigned long bit0_MAX = 3000;

const unsigned long glitch_Length = 300;                  // Anything below this value is a glitch and will be ignored.

// Interrupt variables
unsigned long fall_Time = 0;                              // Placeholder for microsecond time when last falling edge occured.
unsigned long rise_Time = 0;                              // Placeholder for microsecond time when last rising edge occured.
byte bit_Count = 0;                                       // Bit counter for received bits.
unsigned long build_Buffer[] = {0,0};                     // Placeholder last data packet being received.
volatile unsigned long read_Buffer[] = {0,0};             // Placeholder last full data packet read.
volatile byte isrFlags = 0;                               // Various flag bits
volatile unsigned long last_Buffer[] = {0,0};
unsigned long last_Time = 0;

void PinChangeISR(){                                     // Pin 2 (Interrupt 0) service routine
  unsigned long Time = micros();                          // Get current time
  if (digitalRead(ISR_PIN) == LOW) {
// Falling edge
    if (Time > (rise_Time + glitch_Length)) {
// Not a glitch
      Time = micros() - fall_Time;                        // Subtract last falling edge to get pulse time.
      if (bitRead(build_Buffer[1],31) == 1)
        bitSet(isrFlags, F_CARRY_BIT);
      else
        bitClear(isrFlags, F_CARRY_BIT);

      if (bitRead(isrFlags, F_STATE) == 1) {
// Looking for Data
        if ((Time > bit0_MIN) && (Time < bit0_MAX)) {
// 0 bit
          build_Buffer[1] = build_Buffer[1] << 1;
          build_Buffer[0] = build_Buffer[0] << 1;
          if (bitRead(isrFlags,F_CARRY_BIT) == 1)
            bitSet(build_Buffer[0],0);
          bit_Count++;
        }
        else if ((Time > bit1_MIN) && (Time < bit1_MAX)) {
// 1 bit
          build_Buffer[1] = build_Buffer[1] << 1;
          bitSet(build_Buffer[1],0);
          build_Buffer[0] = build_Buffer[0] << 1;
          if (bitRead(isrFlags,F_CARRY_BIT) == 1)
            bitSet(build_Buffer[0],0);
          bit_Count++;
        }
        else {
// Not a 0 or 1 bit so restart data build and check if it's a sync?
          bit_Count = 0;
          build_Buffer[0] = 0;
          build_Buffer[1] = 0;
          bitClear(isrFlags, F_GOOD_DATA);                // Signal data reads dont' match
          bitClear(isrFlags, F_STATE);                    // Set looking for Sync mode
          if ((Time > sync_MIN) && (Time < sync_MAX)) {
            // Sync length okay
            bitSet(isrFlags, F_STATE);                    // Set data mode
          }
        }
        if (bit_Count >= allDataBits) {
// All bits arrived
          bitClear(isrFlags, F_GOOD_DATA);                // Assume data reads don't match
          if (build_Buffer[0] == read_Buffer[0]) {
            if (build_Buffer[1] == read_Buffer[1]) 
              bitSet(isrFlags, F_GOOD_DATA);              // Set data reads match
          }
          read_Buffer[0] = build_Buffer[0];
          read_Buffer[1] = build_Buffer[1];
          last_Buffer[0] = build_Buffer[0];
          last_Buffer[1] = build_Buffer[1];
          last_Time = micros();
          bitSet(isrFlags, F_HAVE_DATA);                  // Set data available
          bitClear(isrFlags, F_STATE);                    // Set looking for Sync mode
digitalWrite(13,HIGH); // Used for debugging
          build_Buffer[0] = 0;
          build_Buffer[1] = 0;
          bit_Count = 0;
        }
      }
      else {
// Looking for sync
        if ((Time > sync_MIN) && (Time < sync_MAX)) {
// Sync length okay
          build_Buffer[0] = 0;
          build_Buffer[1] = 0;
          bit_Count = 0;
          bitSet(isrFlags, F_STATE);                      // Set data mode
digitalWrite(13,LOW); // Used for debugging
        }
      }
      fall_Time = micros();                               // Store fall time
    }
  }
  else {
// Rising edge
    if (Time > (fall_Time + glitch_Length)) {
      // Not a glitch
      rise_Time = Time;                                   // Store rise time
    }
  }
}



void serial_outdoor_dump(void) {


    unsigned long t = micros() - last_Time;
    if ( t > 60 * 1000 * 1000 ) { // 60 s timeout
       Serial.print("# data too old ");
       Serial.println(t);
       return;
    }

    
    unsigned long myData0 = 0;
    unsigned long myData1 = 0;

    // We have at least 2 consecutive matching reads
    myData0 = last_Buffer[0]; // first 4 bits
    myData1 = last_Buffer[1]; // rest of 32 bits
    bitClear(isrFlags,F_HAVE_DATA); // Flag we have read the data

    Serial.print("temp=");
    int temp = ( myData1 >> 8 ) & 0xFFF;
    if ( temp & 0x800 ) temp = temp | 0xF000;
    Serial.print(temp / 10.0, 1);

    Serial.print(" humidity=");
    Serial.print( myData1 & 0xFF );

    byte b1 = ( myData1 >> 20 ); // second byte from received packet

    Serial.print(" button=");
    Serial.print( b1 & 0x04);

    Serial.print(" battery=");
    Serial.print( b1 & 0x08);

    Serial.print(" channel=");
    Serial.print( ( b1 & 0x03 ) + 1 );

    Serial.print(" rid=");
    Serial.println( myData1 >> 24 );

}



// setup

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(0);  // Receiver on inerrupt 0 => that is pin #2  
  mySwitch.enableTransmit(10); // with sender wired in receiving doesn't work, pin #10

  // DS18B20
  sensors.begin();

  // outdoor temperature sensor
  pinMode(13,OUTPUT); // Used for debugging
  pinMode(ISR_PIN,INPUT);
  attachInterrupt(ISR_INT,PinChangeISR,CHANGE);


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

    // light sockets at 513 Mhz
     if (input == 'a') {
       send_513("1000100110110000000000010");
     } else
     if (input == 'b') {
       send_513("1011001001011111000000010");
     } else

     if (input == 'o') {
       serial_outdoor_dump();
     }

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
