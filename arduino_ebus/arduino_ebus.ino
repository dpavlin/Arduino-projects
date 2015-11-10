/*
 * http://ebus-wiki.org/lib/exe/fetch.php/ebus/arduinoebus.pdf
 * 
 * I had to increase values to 47k and 10k on Vaillant boiler so that it doesn't
 * draw 22V line down to 9V
 * 
 * A0 --*--[47k]--- EBUS+
 *      |
 *    [10k]
 *      |
 * GND -*------------ EBUS-     
 * 
 * D13 -> serial RX -> D1
 */

int in=A0;
int out=13;

int treshold = ((9.0*1023.0)/20.02);

void setup() {
  pinMode(in, INPUT);
  pinMode(out, OUTPUT);
//  Serial.begin(2400);
  pinMode(0, INPUT);
  pinMode(1, INPUT);
}

void loop() {
  int inputvalue = analogRead(in);
  if (inputvalue > treshold)
    digitalWrite(out,1);
  else
    digitalWrite(out,0);
}
