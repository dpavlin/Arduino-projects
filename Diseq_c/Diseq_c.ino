/*
 */

// debug on serial
#define DEBUG 1

// pins, 2 recorders 4 inputs each 
int receiver[2][4] = {
	{ A0, A1, A2, A3 },
	{ A4, A5, A6, A7 },
};

#define REC_MASTER 2

#define LED 13

void setup(){
  Serial.begin(9600); // FIXME check speed

  pinMode(REC_MASTER, INPUT_PULLUP); // pullup for jumper/button

  pinMode(receiver[0][0], INPUT);
  pinMode(receiver[0][1], INPUT);
  pinMode(receiver[0][2], INPUT);
  pinMode(receiver[0][3], INPUT);

  pinMode(receiver[1][0], INPUT);
  pinMode(receiver[1][1], INPUT);
  pinMode(receiver[1][2], INPUT);
  pinMode(receiver[1][3], INPUT);

  pinMode(LED, OUTPUT);

#if DEBUG
  Serial.print("boot master=");
  Serial.println(digitalRead(REC_MASTER) == HIGH );
#endif
}

int receiver_selection( int nr ) {
  int StateA = digitalRead( receiver[nr][0] ) == HIGH;
  int StateB = digitalRead( receiver[nr][1] ) == HIGH;
  int StateC = digitalRead( receiver[nr][2] ) == HIGH;
  int StateD = digitalRead( receiver[nr][3] ) == HIGH;

#ifdef DEBUG
  Serial.print("<");
  Serial.print(StateA);
  Serial.print(StateB);
  Serial.print(StateC);
  Serial.print(StateD);
  Serial.print(">");
#endif

  if ( StateA + StateB + StateC + StateD == 0 ) { // no inputs active
    return 0;
  }

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
#define LED_MULTIPLE_INPUTS  125
#define LED_SERIAL           250
#define LED_OFF              0

int current_sat = 0;
int blink_interval = LED_NO_ACTIVE_INPUTS; // 2 sec on/off no receivers turned on


void loop(){

  int nr = 1;  

  while(nr) {
    int sat = receiver_selection(nr-1);

    #if DEBUG
    Serial.print(" r");
    Serial.print(nr);
    Serial.print("=");
    Serial.print(sat);
    Serial.print("|");
    #endif
  
    nr++;
    if (nr > 2) nr = 0;
  
    if ( sat == 0 ) { // slow blink
      blink_interval = LED_NO_ACTIVE_INPUTS;
    } else if ( sat < 0 ) { // error
      blink_interval = LED_MULTIPLE_INPUTS;
    } else {
      blink_interval = LED_OFF;
  
      if ( digitalRead(REC_MASTER) == HIGH ) {
        nr = 0; // stop
        #if DEBUG
        Serial.print("M");
        #endif
      }
  
      if ( current_sat != sat ) {
 
        current_sat = sat;
  
        for(int repeat=0; repeat<2; repeat++) {
  
          Serial.print("@L,");
          Serial.println( char('A' - 1 + sat) );
  
          for(int i=0; i<sat; i++) {
            digitalWrite(LED, HIGH);
            delay(LED_SERIAL);
            digitalWrite(LED, LOW);
            delay(LED_SERIAL);
          }
          delay( 2000 - sat * LED_SERIAL ); // sleep up to 2s
  	//assert(4 * LED_SERIAL < 2000);
        }
      }
    } 
  }


#if DEBUG
  Serial.print(" sat=");
  Serial.print(current_sat);
  Serial.print(" blink=");
  Serial.println(blink_interval);
#endif

  // handle next led blink event in time
  int m = millis() % ( blink_interval * 2 );
  if ( blink_interval != 0 ) {
    if ( m > blink_interval ) {
      digitalWrite(LED, HIGH);
    } else {
      digitalWrite(LED, LOW);
    }
  } else {
    digitalWrite(LED, LOW);
  }
}



