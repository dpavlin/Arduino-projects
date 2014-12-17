#define TX_PIN 7
#define LED_PIN 13

/*
codes for my light sockets:

1000100110110000000000010
1011001001011111000000010

sniffed with rtl-sdr
*/

int int_0 = 300; // ms
int int_1 = 900; // ms
int wait  = 2000; // ms
int repeat = 5; // times

char code[27]; // 26 bits + null

void setup() {
   pinMode(LED_PIN, OUTPUT);
   pinMode(TX_PIN, OUTPUT);
   
   Serial.begin(9600);
   Serial.println("1 or 2 to turn light sockets");
   Serial.println("q/a - 0 inteval +/- 100 ms");
   Serial.println("w/s - 1 inteval +/- 100 ms");
   Serial.println("e/d - wait      +/- 100 ms");
   Serial.println("r/f - repeat    +/- 1");
   Serial.println("U1000100110110000000000010 - send 26 bit user code");
}


void send(char *code) {
  Serial.print("send ");
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

void loop() {
  if(Serial.available()) {
    int in = Serial.read();
    if (in == '1') {
      send("1000100110110000000000010");
    } else if (in == '2') {
      send("1011001001011111000000010");

    } else if (in == 'q') {
      int_0 += 100;
      Serial.print("inteval 0 = ");
      Serial.println(int_0);
    } else if (in == 'a') {
      int_0 -= 100;
      Serial.print("inteval 0 = ");
      Serial.println(int_0);

    } else if (in == 'w') {
      int_1 += 100;
      Serial.print("inteval 1 = ");
      Serial.println(int_1);
    } else if (in == 's') {
      int_1 -= 100;
      Serial.print("inteval 1 = ");
      Serial.println(int_1);

    } else if (in == 'e') {
      wait += 100;
      Serial.print("wait = ");
      Serial.println(wait);
    } else if (in == 'd') {
      wait -= 100;
      Serial.print("wait = ");
      Serial.println(wait);

    } else if (in == 'r') {
      repeat += 1;
      Serial.print("repeat = ");
      Serial.println(repeat);
    } else if (in == 'f') {
      repeat -= 1;
      Serial.print("repeat = ");
      Serial.println(repeat);

    } else if (in == 'U') {
      Serial.readBytesUntil('\n', code, sizeof(code));
      Serial.print('U');
      Serial.println( code );
      send( code );

    } else {
      Serial.print("ignored ");
      Serial.println(in);
    }
  }
}
