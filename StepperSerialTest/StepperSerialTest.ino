// Adafruit Motor shield library
// copyright Adafruit Industries LLC, 2009
// this code is public domain, enjoy!

#include <AFMotor.h>

// Connect a stepper motor with 48 steps per revolution (7.5 degree)
// to motor port #2 (M3 and M4)
AF_Stepper motor(48, 2);

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Stepper test!");

  motor.setSpeed(10);  // 10 rpm   
}

void loop() {
//  Serial.println("Single coil steps");
//  motor.step(100, FORWARD, SINGLE); 
//  motor.step(100, BACKWARD, SINGLE); 

  if (Serial.available()) {
    int in = Serial.read();
    Serial.println(in, DEC);
    if ( in == 102 ) { // f
      Serial.println("Double coil steps forward");
      motor.step(48, FORWARD, DOUBLE);
    } else if ( in == 98 ) { // b
      Serial.println("Double coil steps backward");
      motor.step(48, BACKWARD, DOUBLE);
    } else if ( in >= 49 && in <= 57 ) {
      int speed = ( in - 48 ) * 10;
      Serial.print("speed ");
      Serial.print(speed, DEC);
      Serial.println(" rpm");
      motor.setSpeed( speed );
    }
  }

//  Serial.println("Interleave coil steps");
//  motor.step(100, FORWARD, INTERLEAVE); 
//  motor.step(100, BACKWARD, INTERLEAVE); 

//  Serial.println("Micrsostep steps");
//  motor.step(100, FORWARD, MICROSTEP); 
//  motor.step(100, BACKWARD, MICROSTEP); 
}
