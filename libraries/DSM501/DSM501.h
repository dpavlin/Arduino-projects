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
/**
see also https://github.com/alexjx/AqiMon/blob/master/DSM501.h
             http://www.guokr.com/article/434130/
             http://www.geek-workshop.com/thread-3829-1-1.html
             
*/
#ifndef DSM501_H
#define DSM501_H
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#define MIN_WIN_SPAN	30	// 30S
#define MAX_WIN_SPAN     300	// 300S

#define PM10_PIN	3
#define PM25_PIN	4

#define PM10_IDX	0
#define PM25_IDX	1

class DSM501 {
public:
	DSM501(int pin10 = PM10_PIN, int pin25 = PM25_PIN);
	void begin(uint32_t span = MIN_WIN_SPAN);
	void update(); // called in the loop function for update
	void reset();

	float getLowRatio(int i = 0);
	float getParticalWeight(int i = 0);
	float getPM25();
	uint32_t getAQI();

private:
	int _pin[2];
	uint32_t _low_total[2];
	uint32_t _win_total[2];
	uint32_t _span;
	uint8_t _done[2];
	float _lastLowRatio[2];
};

#endif
