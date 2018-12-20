/*
 * power cycle switch
 * 
 * relay is connected across 5V relay through normally closed pins so that failure of arduino doesn't kill power to switch.
 * to activate relay on this board, signal pin has to be pulled to ground.and coil draw is 76 mA when active
 * board has pull up on input pin to it's vcc
*/

#define RELAY_PIN 2

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(RELAY_PIN, INPUT); // don't modify pin state
  Serial.print("Relay pin on reset: ");
  Serial.println(digitalRead(RELAY_PIN));
}

void loop() {
  if ( Serial.available() ) {
    char c = Serial.read();
    if ( c == '0' ) {
      Serial.print("L");
      pinMode(RELAY_PIN, OUTPUT);
      digitalWrite(RELAY_PIN, LOW); // activate relay

      digitalWrite(LED_BUILTIN, HIGH); // led on
    } else if ( c == '1' ) {
      Serial.print("H");
      pinMode(RELAY_PIN, INPUT);

      digitalWrite(LED_BUILTIN, LOW); // led off
    } else {
      Serial.print(c);
    }
  }
}
