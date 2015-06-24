/*
 * A simple single freq AD9850 Arduino test script
 * Original AD9851 DDS sketch by Andrew Smallbone at www.rocketnumbernine.com
 * Modified for testing the inexpensive AD9850 ebay DDS modules
 * Pictures and pinouts at nr8o.dhlpilotcentral.com
 * 9850 datasheet at http://www.analog.com/static/imported-files/data_sheets/AD9850.pdf
 * Use freely
 */
 
#define W_CLK 7	// connect to AD9850 module word load clock pin (CLK)
#define FQ_UD 6	// connect to freq update pin (FQ)
#define DATA  5	// connect to serial data load pin (DATA)
#define RESET 4	// connect to reset pin (RST).

#define ENCODER 0
#define encoder_a 7
#define encoder_b 8
#define encoder_click 9
 
#define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW); }

double user_freq = 1.e6; // 10.e6
 
// transfers a byte, a bit at a time, LSB first to the 9850 via serial DATA line
void tfr_byte(byte data)
{
  for (int i=0; i<8; i++, data>>=1) {
    digitalWrite(DATA, data & 0x01);
    pulseHigh(W_CLK);   //after each bit sent, CLK is pulsed high
  }
}

 
// frequency calc from datasheet page 8 = <sys clock> * <frequency tuning word>/2^32
void sendFrequency(double frequency) {
  int32_t freq = frequency * 4294967295/125000000;  // note 125 MHz clock on 9850
  for (int b=0; b<4; b++, freq>>=8) {
    tfr_byte(freq & 0xFF);
  }
  tfr_byte(0x000);   // Final control byte, all 0 for 9850 chip
  pulseHigh(FQ_UD);  // Done!  Should see output
  
  Serial.print("Freq = ");
  Serial.print((frequency / 1.e6), 4);
  Serial.println(" MHz");
}

 
void setup() {
 // configure arduino data pins for output
  pinMode(FQ_UD, OUTPUT);
  pinMode(W_CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT);
 
  pulseHigh(RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD);  // this pulse enables serial mode - Datasheet page 12 figure 10

#if ENCODER
  // encoder
  pinMode(encoder_a, INPUT);
  pinMode(encoder_b, INPUT);
  pinMode(encoder_click, INPUT);
#endif
  
  Serial.begin(9600);
  sendFrequency(user_freq);
}

double freq_step = 1.e5; // 0.1 MHz
int encoder_state = LOW;
int encoder_last  = LOW;

double new_freq = 0;

void loop() {
  if (Serial.available()) {
     int inByte = Serial.read();
     Serial.println(inByte, DEC);
     if (inByte == 43 || inByte == 61 or inByte == 67) { // + = left
       user_freq += freq_step;
       sendFrequency(user_freq);
     } else if ( inByte == 45 || inByte == 68) { // - right
       user_freq -= freq_step; 
       sendFrequency(user_freq);
     } else if ( inByte == 65 ) { // up
       user_freq -= freq_step * 10; 
       sendFrequency(user_freq);
     } else if ( inByte == 66 ) { // down
       user_freq += freq_step * 10; 
       sendFrequency(user_freq);
     } else if ( inByte >= 48 && inByte <= 57) { // 0 .. 9
       user_freq = ( inByte - 48 ) * 1.e6; 
       sendFrequency(user_freq);
     } else if ( inByte == 13 ) { // enter
       Serial.print("enter Mhz = ");
       while (Serial.available() == 0);
       new_freq = Serial.parseFloat();
       while (Serial.available() > 0) { Serial.read(); } // suck enter and just from partFloat
       Serial.println();
       if ( new_freq == 0 ) {
          Serial.println("# ignored");
       } else {
           user_freq = new_freq * 1.e6;
           sendFrequency(user_freq);
       }
     }
  }

#if ENCODER
  encoder_state = digitalRead(encoder_a);
  if ((encoder_last == LOW) && (encoder_state == HIGH)) {
    if (digitalRead(encoder_b) == LOW) {
      user_freq -= freq_step;
    } else {
      user_freq += freq_step;
    }
    sendFrequency(user_freq);
  }
  encoder_last = encoder_state;
  
  if(digitalRead(encoder_click) == LOW) sendFrequency( 1.e6 ); // reset to 1MHz
#endif

}
