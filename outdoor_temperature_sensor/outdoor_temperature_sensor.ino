// Unknown 433Mhz weather sensor decoder. Untested in the real world.
// http://arduino.cc/forum/index.php/topic,142871.msg1106336.html#msg1106336

// __           ___       ___    ___
//   |         |  |      |  |   |  |
//   |_________|  |______|  |___|  |
//
//   |  Sync      |    1    |  0   |
//   |  8320us    | 4500us  | 2530us

// Defines
#define allDataBits 42                                    // Number of data bits to expect
// isrFlags bit numbers
#define F_HAVE_DATA 1                                     // 0=Nothing in read buffer, 1=Data in read buffer
#define F_GOOD_DATA 2                                     // 0=Unverified data, 1=Verified (2 consecutive matching reads)
#define F_CARRY_BIT 3                                     // Bit used to carry over bit shift from one long to the other
#define F_STATE 7                                         // 0=Sync mode, 1=Data mode

// Constants
const unsigned long sync_MIN = 8120;                      // Minimum Sync time in micro seconds
const unsigned long sync_MAX = 8520;
const unsigned long bit1_MIN = 4300;
const unsigned long bit1_MAX = 4700;
const unsigned long bit0_MIN = 2330;
const unsigned long bit0_MAX = 2730;
const unsigned long glitch_Length = 300;                  // Anything below this value is a glitch and will be ignored.

// Interrupt variables
unsigned long fall_Time = 0;                              // Placeholder for microsecond time when last falling edge occured.
unsigned long rise_Time = 0;                              // Placeholder for microsecond time when last rising edge occured.
byte bit_Count = 0;                                       // Bit counter for received bits.
unsigned long build_Buffer[] = {0,0};                     // Placeholder last data packet being received.
volatile unsigned long read_Buffer[] = {0,0};             // Placeholder last full data packet read.
volatile byte isrFlags = 0;                               // Various flag bits

void PinChangeISR0(){                                     // Pin 2 (Interrupt 0) service routine
  unsigned long Time = micros();                          // Get current time
  if (digitalRead(2) == LOW) {
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


void setup() {
pinMode(13,OUTPUT); // Used for debugging
  Serial.begin(9600);
  pinMode(2,INPUT);
  Serial.println(F("ISR Pin 2 Configured For Input."));
  attachInterrupt(0,PinChangeISR0,CHANGE);
  Serial.println(F("Pin 2 ISR Function Attached. Here we go."));
}

void loop() {
  unsigned long myData0 = 0;
  unsigned long myData1 = 0;
  if (bitRead(isrFlags,F_GOOD_DATA) == 1) {
    // We have at least 2 consecutive matching reads
    myData0 = read_Buffer[0]; // Read the data spread over 2x 32 variables
    myData1 = read_Buffer[1];
    bitClear(isrFlags,F_HAVE_DATA); // Flag we have read the data
    dec2binLong(myData0,10);
    dec2binLong(myData1,32);

    Serial.print(" - Battery=");
    byte H = (myData1 >> 30) & 0x3;   // Get Battery
    Serial.print(H);
    Serial.print(" Channel=");
    H = (myData1 >> 28) & 0x3;        // Get Channel
    Serial.print(H);
    Serial.print(" Temperature=");
    byte ML = (myData1 >> 16) & 0xF0; // Get MMMM
    H = (myData1 >> 24) & 0xF;        // Get LLLL
    ML = ML | H;                      // OR MMMM & LLLL nibbles together
    H = (myData1 >> 16) & 0xF;        // Get HHHH
    int Temperature = (H << 8) | ML;  // Combine HHHH MMMMLLLL
    Temperature = Temperature -900;   // Remove Constant offset
    Serial.print(Temperature/10.0,1);
    Serial.print("F Humidity=");
    H = (myData1 >> 4) & 0xF0;        // Get HHHH
    ML = (myData1 >> 12) & 0xF;       // Get LLLL
    ML = ML | H;                      // OR HHHH & LLLL nibbles together
    Serial.print(ML);
    Serial.println("%");
  }
  delay(100);
}

void dec2binLong(unsigned long myNum, byte NumberOfBits) {
  if (NumberOfBits <= 32){
    myNum = myNum << (32 - NumberOfBits);
    for (int i=0; i<NumberOfBits; i++) {
      if (bitRead(myNum,31) == 1) 
      Serial.print("1");
      else 
      Serial.print("0");
      myNum = myNum << 1;
    }
  }
}

