/**
The MIT License (MIT)

Copyright (c) 2019 Sovichea Tep

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "DSM501.h"

#define DEBUG 0

volatile uint32_t _low_total[2];
volatile uint32_t _t_ellapse[2];
volatile uint8_t _state_prev[2];
int _pin[2];
uint32_t _span;

static void pulseMeasure(int pm_index, int pulse_state) 
{
	if (digitalRead(_pin[pm_index]) == pulse_state)
	{
		_t_ellapse[pm_index] = micros();
		_state_prev[pm_index] = pulse_state;
	}
	else
	{
		if (_state_prev[pm_index] == pulse_state)
		{
			_t_ellapse[pm_index] = (uint32_t)(micros() - _t_ellapse[pm_index]);
			_low_total[pm_index] += _t_ellapse[pm_index];
			_state_prev[pm_index] = !pulse_state;
#if DEBUG
			Serial.print("PM10_low: ");
			Serial.print(_low_total[pm_index]);
			Serial.println(" us");
#endif
		}
	}
}

#if defined(ESP32) || defined(ESP8266)
void ICACHE_RAM_ATTR PM10_handleInterrupt()
#else
void PM10_handleInterrupt()
#endif
{
	pulseMeasure(PM10_IDX, LOW);
};

#if defined(ESP32) || defined(ESP8266)
void ICACHE_RAM_ATTR PM25_handleInterrupt()
#else
void PM25_handleInterrupt()
#endif
{
	pulseMeasure(PM25_IDX, LOW);
};

void DSM501::begin(int pin10, int pin25, uint32_t span) 
{
#if DEBUG
	Serial.println("Initializing DSM501...");
#endif

	_pin[PM10_IDX] = pin10;
	_pin[PM25_IDX] = pin25;

	pinMode(_pin[PM10_IDX], INPUT);
	pinMode(_pin[PM25_IDX], INPUT);

	for (int i = 0; i < 2; i++) 
	{
		_low_total[i] = 0;
		_lastLowRatio[i] = -1;
	}

	_update_start = 1;
	_span = span * 1000;
}

uint8_t DSM501::update() 
{
	if (_update_start)
	{
#if DEBUG
		Serial.println("Updating sensor reading...");
#endif
		_update_done = 0;
		_update_start = 0;
		_starttime = millis();

		for (int i = 0; i < 2; i++) 
		{
			_low_total[i] = 0;
		}
		
		attachInterrupt(digitalPinToInterrupt(_pin[PM10_IDX]), PM10_handleInterrupt, CHANGE);
		attachInterrupt(digitalPinToInterrupt(_pin[PM25_IDX]), PM25_handleInterrupt, CHANGE);
	}
	else
	{
		if ((uint32_t)(millis() - _starttime) >= _span)
		{
			_update_done = 1;
			_update_start = 1;
			detachInterrupt(_pin[PM10_IDX]);
			detachInterrupt(_pin[PM25_IDX]);

			// ratio = low pulse (microsecond) * 100 / (sample time * 1000)
			_lastLowRatio[PM10_IDX] = _low_total[PM10_IDX] / (_span * 10.0);
			_lastLowRatio[PM25_IDX] = _low_total[PM25_IDX] / (_span * 10.0);
		}
	}
	return _update_done;
}

float DSM501::getLowRatio(int i) 
{
	return _lastLowRatio[i];
}

float DSM501::getConcentration()
{
	/* The estimation for dust density and radius below is taken from this paper
	* https://www.researchgate.net/publication/216680944_Estimation_of_particle_mass_concentration_in_ambient_air_using_a_particle_counter
	* 
	* assume all particles are spherical with density of 1.65x10^12 ug/m^3 
	* assume the radius of each particle in PM2.5 channel is 0.44 um
	* 
	* volume of a sphere = 4/3 * pi * raduis^3
	*                    = 3.5682E-19 m^3
	* 
	* mass = density * volume;
	*      = 5.8875E-07 ug
	* 
	* parts/m3 = parts/283mL * 1E6/283
	* ug/m3    = parts/m3 * mass
	*          = parts/283mL * 0.002179
	*/

	long particle_count_PM25 = getParticleCount(0) - getParticleCount(1);
	float concentration = particle_count_PM25 * 0.002179;

	return concentration  < 0.0 ? 0.0 : concentration;
}

long DSM501::getParticleCount(int i) {
	/*
	 * regression function is derived from the specs sheet below
	 * https://github.com/nateGeorge/ESP-8266-particle-sensor/blob/master/spec%20sheets/DSM501%20spec%20sheet.pdf
	 * and use third order polynomial to fit the data
	 * 
	 *    parts/283mL = 0.5831 * r^3 - 15.924 * r^2 + 729.37 * r - 82.523
	 */

	float r = getLowRatio(i);
	long particle_count = 0.5831 * pow(r, 3) - 15.924 * pow(r, 2) + 729.37 * r - 82.523;
	return particle_count  < 0.0 ? 0.0 : particle_count;
}
