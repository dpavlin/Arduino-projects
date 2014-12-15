#define TX_PIN 7
#define LED_PIN 13

/*
codes for my light sockets:

1000100110110000000000010
1011001001011111000000010

sniffed with rtl-sdr
*/

void setup() {
   pinMode(LED_PIN, OUTPUT);
   pinMode(TX_PIN, OUTPUT);
   
   Serial.begin(9600);
   Serial.println("1 or 2 to turn light sockets");
}


void send(char *code) {
  Serial.print("send ");
  Serial.println(code);
  
  // we have to send same signal at least two times
  for(int repeat = 0; repeat < 5; repeat++ ) {
    
    digitalWrite(LED_PIN, HIGH);

    for(int i = 0; i < strlen(code); i++) {
      int i1 = 300;
      int i2 = 900;
      if (code[i] == '1' ) {
        i1 = 900;
        i2 = 300;
      }
      digitalWrite(TX_PIN, HIGH);
      delayMicroseconds(i1);
      digitalWrite(TX_PIN, LOW);
      delayMicroseconds(i2);
    }

    delayMicroseconds(2000); // guess
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
    } else {
      Serial.print("ignored ");
      Serial.println(in);
    }
  }
}
