/**
The MIT License (MIT)

Copyright (c) 2015 richardhmm

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

DSM501::DSM501(int pin10, int pin25) {
	_pin[PM10_IDX] = pin10;
	_pin[PM25_IDX] = pin25;

	for (int i = 0; i < 2; i++) {
		_win_total[i] = 0;
		_low_total[i] = 0;
		_lastLowRatio[i] = -1;
		_done[i] = 0;
	}
	
}

void DSM501::begin(uint32_t span) {
	pinMode(_pin[PM10_IDX], INPUT);
	pinMode(_pin[PM25_IDX], INPUT);
		
	if (span < MIN_WIN_SPAN)
	    _span = MIN_WIN_SPAN * 1000;
	else if (span > MAX_WIN_SPAN)
	    _span = MAX_WIN_SPAN * 1000;
	else
	    _span = span * 1000;
}

void DSM501::reset() {
	for (int i = 0; i < 2; i++) {
		_win_total[i] = 0;
		_low_total[i] = 0;
		_done[i] = 0;
	}
}

void DSM501::update() {
	uint32_t start = millis();
	for(;;)
	{
		if (millis() < start || millis() - start >= _span)
		{
		    _done[0] = 1;
		    _done[1] = 1;
		    break;
		}
              _win_total[0] ++;
		_low_total[0] += !digitalRead(_pin[PM10_IDX]);
		_win_total[1] ++;
		_low_total[1] += !digitalRead(_pin[PM25_IDX]);
	}
}

float DSM501::getLowRatio(int i) {
	if (_done[i] == 1)
	{
	    _lastLowRatio[i] = (_low_total[i] * 100.0) / _win_total[i];
	    _low_total[i] = 0;
	    _win_total[i] = 0;
	    _done[i] = 0;
	    return _lastLowRatio[i];
	}
	else
	    return _lastLowRatio[i];
}


float DSM501::getParticalWeight(int i) {
	/*
	 * with data sheet...regression function is
	 *    y=0.1776*x^3-2.24*x^2+ 94.003*x
	 */
	float r = getLowRatio(i);
	float weight = 0.1776*pow(r,3) - 0.24*pow(r,2) + 94.003*r;
	return weight  < 0.0 ? 0.0 : weight;
}

float DSM501::getPM25() {
	return getParticalWeight(0) - getParticalWeight(1);
}

// China pm2.5 Index
uint32_t DSM501::getAQI() {
	// this works only under both pin configure
	uint32_t aqi = 0;

	float P25Weight = getPM25();
	  if (P25Weight>= 0 && P25Weight <= 35) {
	    aqi = 0   + (50.0 / 35 * P25Weight);
	  } 
	  else if (P25Weight > 35 && P25Weight <= 75) {
	    aqi = 50  + (50.0 / 40 * (P25Weight - 35));
	  } 
	  else if (P25Weight > 75 && P25Weight <= 115) {
	    aqi = 100 + (50.0 / 40 * (P25Weight - 75));
	  } 
	  else if (P25Weight > 115 && P25Weight <= 150) {
	    aqi = 150 + (50.0 / 35 * (P25Weight - 115));
	  } 
	  else if (P25Weight > 150 && P25Weight <= 250) {
	    aqi = 200 + (100.0 / 100.0 * (P25Weight - 150));
	  } 
	  else if (P25Weight > 250 && P25Weight <= 500) {
	    aqi = 300 + (200.0 / 250.0 * (P25Weight - 250));
	  } 
	  else if (P25Weight > 500.0) {
	    aqi = 500 + (500.0 / 500.0 * (P25Weight - 500.0)); // Extension
	  } 
	  else {
	    aqi = 0; // Initializing
	  }

	return aqi;
}
