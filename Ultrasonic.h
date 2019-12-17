
 //Ultrasonic.h
 //A library for ultrasonic ranger

#pragma once
#include "nrf.h"
#include <stdbool.h>
#include <stdint.h>


//The measured distance from the range 0 to 400 Centimeters
long MeasureInCentimeters(void);

//The measured distance from the range 0 to 157 Inches
long MeasureInInches(void);

uint32_t MicrosDiff(uint32_t begin, uint32_t end);

uint32_t pulseIn(uint32_t pin, uint32_t state);

