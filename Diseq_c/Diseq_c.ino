/*
pins, 2 recorders 4 inputs each 
 */

#define REC_1_A A0
#define REC_1_B A1
#define REC_1_C A2
#define REC_1_D A3

#define REC_2_A A4
#define REC_2_B A5
#define REC_2_C A6
#define REC_2_D A7

#define REC_MASTER 2

#define LED 13

void setup(){
  Serial.begin(9600); // FIXME check speed

  pinMode(REC_MASTER, INPUT_PULLUP); // pullup for jumper/button

  pinMode(REC_1_A, INPUT);
  pinMode(REC_1_B, INPUT);
  pinMode(REC_1_C, INPUT);
  pinMode(REC_1_D, INPUT);

  pinMode(REC_2_A, INPUT);
  pinMode(REC_2_B, INPUT);
  pinMode(REC_2_C, INPUT);
  pinMode(REC_2_D, INPUT);

  pinMode(LED, OUTPUT);

}

int recorder_inputs( int PinA, int PinB, int PinC, int PinD ) {
  int StateA = digitalRead( PinA );
  int StateB = digitalRead( PinB );
  int StateC = digitalRead( PinC );
  int StateD = digitalRead( PinD );
  
  if ( StateA + StateB + StateC + StateD != 1 ) { // only one active at a time
    return -1; // error
  }

  if ( StateA ) {
    return 1;
  } else if ( StateB ) {
    return 2;
  } else if ( StateC ) {
    return 3;
  } else if ( StateD ) {
    return 4;
  }

  return 0; // receiver off
}

#define LED_NO_ACTIVE_INPUTS 2000
#define LED_ERROR            125
#define LED_SERIAL           250
#define LED_OFF              0

int current_L = 0;
int blink_interval = LED_NO_ACTIVE_INPUTS; // 2 sec on/off no receivers turned on
int next_blink = 0;


void loop(){
  
  int sat = recorder_inputs( REC_1_A, REC_1_B, REC_1_C, REC_1_D );
  int try_slave = 1;

again_for_slave:
  if ( sat < 0 ) { // error
    blink_interval = LED_ERROR;
  } else if ( sat == 0 ) {
    blink_interval = LED_NO_ACTIVE_INPUTS;
  } else {

    if ( digitalRead(REC_MASTER) == HIGH ) try_slave = 0;

    if ( current_L != sat ) {

      blink_interval = LED_OFF;

      for(int repeat=0; repeat<2; repeat++) {

        Serial.print("@L,");
        Serial.println( 'A' - 1 + sat );

        for(int i=0; i<sat; i++) {
          digitalWrite(LED, HIGH);
          delay(LED_SERIAL);
          digitalWrite(LED, LOW);
          delay(LED_SERIAL);
        }
        delay( 2000 - sat * LED_SERIAL ); // sleep up to 2s
      }

    } 
  }

  if ( try_slave == 1 ) {
    try_slave = 0;
    sat = recorder_inputs( REC_2_A, REC_2_B, REC_2_C, REC_2_D );
    goto again_for_slave;
  }

  // handle next led blink event in time
  int m = micros();
  if ( blink_interval != 0 && m > next_blink ) {
    digitalWrite(LED, ! digitalRead(LED));
    next_blink = m + blink_interval;
  }

}



