# DSM501 Interrupt
This library aims to implement the reading of DSM501 PWM pulse using external interrupt. It has been tested on Arduino Uno and ESP8266. 



## Hardware connection

Both sensor pins must be connected to interrupt pin on your desired board. For Arduino Uno, there are only two interrupt pins, which are on pin D2 and pin D3.

On the other hand, all pins on ESP8266 can be used for interrupt event **except for D4**, which cause the board to reset.



## Code usage

First DSM501 object must be instantiated as shown below

`DSM501 dsm501;`

After that, the object is initialized with 3 parameters: PM1.0 pin, PM2.5 pin and a desired sample time

`dsm501.begin(DSM501_PM1_0, DSM501_PM2_5, SAMPLE_TIME);`

A piece of code is included in the example to delay for 60 seconds to wait for the heater in the module to heat up and stabilized before reading the result.

```c
// Wait 60s for DSM501 to warm up
Serial.println("Wait 60s for DSM501 to warm up"); 
for (int i = 1; i <= 60; i++)
{
    delay(1000); // 1s
    Serial.print(i);
    Serial.println(" s (wait 60s for DSM501 to warm up)");
}
Serial.println("DSM501 is ready!");
```

Once the initialization is compete `dsm501.update()` is used to check if the sample time is reached and ready for the sensor value to be read.

```c
if (dsm501.update())
{
    long particleCountPM1_0 = dsm501.getParticleCount(0);
    long particleCountPM2_5 = dsm501.getParticleCount(1);
    float concentration = dsm501.getConcentration();
}
```

`getParticleCount(int i)` is written specifically to get the number of particle counts in *parts/283mL*​ or *parts/0.1cf*. Index **0** is the number of counts for any particle larger than PM1.0, Index **1** is the number of counts for any particle larger than PM2.5.

`getConcentration()` uses the number of particle counts between PM1.0 and PM2.5, then estimates the concentration in  `ug/m^3`. The estimation is based on a Journal on Atmospheric Environment (<https://www.researchgate.net/publication/216680944_Estimation_of_particle_mass_concentration_in_ambient_air_using_a_particle_counter>). It is done by assuming that all particles are **spherical** with an average radius `r = 0.44um` and a density `D = 1.65x10^12ug/m^3`. 

