/*
 */

// debug on serial
#define DEBUG 1

// serial commands for loop-back test
#define TEST 1

// pins, 2 receivers 4 inputs each 
int receiver[2][4] = {
  { A0, A1, A2, A3 },
  { A4, A5, A6, A7 },
};

#define REC_MASTER 2

#define LED 13

//                  rec1        rec2     master
int test_pins[] = { 12,11,10,9, 8,7,6,5, 4 };
int test_out[]  = {  0, 0, 0,0, 0,0,0,0, 1};
int nr_test_pins = sizeof(test_pins)/sizeof(int);

int master_enabled() {
  return digitalRead(REC_MASTER) == HIGH ? 1 : 0;
}

void setup(){
  Serial.begin(9600); // FIXME check speed

  pinMode(REC_MASTER, INPUT_PULLUP); // pullup for jumper/button

  for(int nr=0; nr<2; nr++) {
    for(int input=0; input<4; input++) {
      pinMode(receiver[nr][input], INPUT);
    }
  }

  pinMode(LED, OUTPUT);

#if DEBUG
  Serial.print("boot master=");
  Serial.println(master_enabled());
#endif

#if TEST
  for(int i=0;i<nr_test_pins;i++) {
    pinMode(test_pins[i], OUTPUT);
    digitalWrite(test_pins[i], test_out[i]);
  }
#endif
}

char restore[10] = "";
char len = 0;

int receiver_selection( int nr ) {

  #ifdef DEBUG
  Serial.print("|<");
  #endif

  if ( nr == 0 ) {
    len = 0;
    if ( ! master_enabled() ) restore[len++] = '9';
  }

  int active = 0;
  int selected = -1;

  for(int i=0; i<4; i++) {
    int a = analogRead( receiver[nr][i] );
    int on_off = a > 512 ? 1 : 0;
    active += on_off;
    if ( on_off ) {
      selected = i+1;
      restore[len++] = char('1' + (nr * 4) + i);
    }

    //Serial.print(a);
    //Serial.print("~");
    Serial.print(on_off);
    //Serial.print(" ");
  }
  restore[len] = 0;

  Serial.print(">");

  if ( active == 0 ) { // no inputs active
    return 0;
  }

  if ( active != 1 ) { // only one active at a time
    return -1; // error
  }

  Serial.print(" selected=");
  Serial.print(selected);

  return selected;
}

#define LED_NO_ACTIVE_INPUTS 2000
#define LED_MULTIPLE_INPUTS  125
#define LED_SERIAL           200 // < 250
#define LED_OFF              0

int current_sat = 0;
int blink_interval = LED_NO_ACTIVE_INPUTS; // 2 sec on/off no receivers turned on
int last_changed_nr = -1;
int last_receiver_selection[2] = { -1, -1 };

void loop(){

  int nr = 1;

  while(nr && nr <= 2) {
    int i = nr - 1;

    int current_nr = nr;

    int sat = receiver_selection(i);
    int last_sat = last_receiver_selection[i];
    last_receiver_selection[i] = sat;

    #if DEBUG
    Serial.print(" nr:");
    Serial.print(nr);
    Serial.print("=");
    Serial.print(sat);
    Serial.print(" last:");
    Serial.print(last_sat);
    #endif

    int prefer_master = master_enabled();
    if ( ! prefer_master ) {

      if ( last_sat != sat ) {

        Serial.print("C");
        last_changed_nr = current_nr;
        if ( sat == 0 ) {
          Serial.print("O");
          sat = receiver_selection( (i+1)%2 ); // check other receiver
        }
        nr=0; // stop

      } else {

        Serial.print("=");
        nr++; // next
        continue;

      }

      if ( last_changed_nr != current_nr ) {
        sat = current_sat; // ignore, last not changed
        Serial.print("I");
      }

    }
 
    if ( sat == 0 ) { // slow blink
      blink_interval = LED_NO_ACTIVE_INPUTS;
      nr++; // next
    } else if ( sat < 0 ) { // error
      blink_interval = LED_MULTIPLE_INPUTS;
      nr = 0;
    } else {
      blink_interval = LED_OFF;
  
      if ( prefer_master ) {
        #if DEBUG
        Serial.print("[M]");
        #endif
        if ( nr == 1 ) { // need to check 2nd receiver error and report it
          int error = receiver_selection(i + 1);
          Serial.print(" E?");
          Serial.print(error);
          if ( error == -1 ) blink_interval = LED_MULTIPLE_INPUTS;
        }
        nr = 0; // stop
      } else {
        if ( last_sat != sat ) {
          Serial.print("[C]");
          last_changed_nr = current_nr;
          nr=0;
        } else {
          Serial.print("[S]");
          nr++;
        }

        if ( last_changed_nr != current_nr ) {
          sat = current_sat; // ignore, last not changed
          Serial.print("[I]");
        }
  
      }

      if ( current_sat != sat ) {

        last_changed_nr = current_nr;
        current_sat = sat;
        nr = 0; // stop

        for(int repeat=0; repeat<2; repeat++) {
  
          Serial.print("@L,");
          Serial.println( char('A' + sat - 1) );
  
          for(int i=0; i<sat; i++) {
            digitalWrite(LED, HIGH);
            delay(LED_SERIAL);
            digitalWrite(LED, LOW);
            delay(LED_SERIAL);
          }
          delay( ( 5 - sat ) * LED_SERIAL * 2 ); // sleep up to 2s
    //assert(4 * LED_SERIAL < 2000);
        }

      }

    } // while

  }


  #if DEBUG
  Serial.print("| sat=");
  Serial.print(current_sat);
  Serial.print(" from:");
  Serial.print(last_changed_nr);
  Serial.print(" restore>");
  Serial.print(restore);
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

  #if TEST
  while ( Serial.available() ) {
    int serial = Serial.read();
    Serial.print(serial, HEX);
    if ( serial >= '1' && serial <= ( '1' + nr_test_pins ) ) {
      int nr = serial - '1';
      Serial.print(" toggle pin ");
      test_out[nr] = ! test_out[nr];
      int pin = test_pins[nr];
      Serial.print(pin);
      Serial.print("=");
      int out = test_out[nr];
      Serial.print(out);
      digitalWrite( pin, out );

    }
    Serial.print(" <");
    for(int i=0;i<nr_test_pins;i++) {
      if ( i == 4 || i == 8 ) Serial.print(" ");
      Serial.print(test_out[i]);
    }
    Serial.println(">");
  }
  #endif
}

// vim: tabstop=2 shiftwidth=2 expandtab
