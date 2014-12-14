#define TX_PIN 7
#define LED_PIN 13

char *code = "1000100110110000000000010";
//char *code = "1011001001011111000000010";

void setup() {
   pinMode(LED_PIN, OUTPUT);
   pinMode(TX_PIN, OUTPUT);
}

void loop() {
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
  
  digitalWrite(LED_PIN, LOW);  
  delay(3000);
}
