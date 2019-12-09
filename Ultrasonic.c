/*
 * Ultrasonic.c
 * A library for ultrasonic ranger, modified for the Buckler
 *
 * Copyright (c) 2012 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : LG, FrankieChu
 * Create Time: Jan 17,2013
 * Change Log : Nov 20,2019
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <stdio.h>
#include <string.h>

#include <gpio.h>
#include <virtual_timer.h>
#include "nrf.h"
#include "nrf_delay.h"
#include <inttypes.h>
uint32_t pin = 4;

static uint32_t MicrosDiff(uint32_t begin, uint32_t end)
{
	return end - begin;
}

static uint32_t pulseIn(uint32_t pin, uint32_t state)
{
  uint32_t timeout = 1000000L;
	uint32_t begin = read_timer();
	
	// wait for any previous pulse to end
	while (gpio_read(pin)) if (MicrosDiff(begin, read_timer()) >= timeout) return 0;
	
	// wait for the pulse to start
	while (!gpio_read(pin)) if (MicrosDiff(begin, read_timer()) >= timeout) return 0;
	uint32_t pulseBegin = read_timer();
	
	// wait for the pulse to stop
	while (gpio_read(pin)) if (MicrosDiff(begin, read_timer()) >= timeout) return 0;
	uint32_t pulseEnd = read_timer();
	
	return MicrosDiff(pulseBegin, pulseEnd);
}







/*The measured distance from the range 0 to 400 Centimeters*/
long MeasureInCentimeters(void)
{
	gpio_config(pin, OUTPUT);
	gpio_clear(pin);
	nrf_delay_ms(2);
	gpio_set(pin);
	nrf_delay_ms(5);	
	gpio_clear(pin);
	gpio_config(pin, INPUT);
	long duration;
	duration = pulseIn(pin,1);
	long RangeInCentimeters;
	RangeInCentimeters = duration/29/2;
	return RangeInCentimeters;
}
/*The measured distance from the range 0 to 157 Inches*/
long MeasureInInches(void)
{
	gpio_config(pin, OUTPUT);
	gpio_clear(pin);
	nrf_delay_ms(2);
	gpio_set(pin);
	nrf_delay_ms(5);	
	gpio_clear(pin);
	gpio_config(pin, INPUT);
	long duration;
	duration = pulseIn(pin,1);
	long RangeInInches;
	RangeInInches = duration/74/2;
	return RangeInInches;
}
