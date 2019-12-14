#include "gpio.h"

typedef struct {
 uint32_t OUT;
 uint32_t OUTSET;
 uint32_t OUTCLR;
 uint32_t IN;
 uint32_t DIRECT;
 uint32_t DIRSET;
 uint32_t DIRCLR;
 uint32_t LATCH;
 uint32_t DETECTMODE;
 uint32_t reserved[118];
 uint32_t PIN_CNF[32];
} gpio_struct_t;

gpio_struct_t* gpio = 0x50000504;

// Inputs: 
//  gpio_num - gpio number 0-31
//  dir - gpio direction (INPUT, OUTPUT)
void gpio_config(uint8_t gpio_num, gpio_direction_t dir) {
  if (dir == OUTPUT) {
    gpio->DIRECT |= 1 << gpio_num;
  } else {
    gpio->PIN_CNF[gpio_num] = 0x00020000;
  }
}

// Set gpio_num high
// Inputs: 
//  gpio_num - gpio number 0-31
void gpio_set(uint8_t gpio_num) {
  gpio->OUTSET = 1 << gpio_num;
}

// Set gpio_num low
// Inputs: 
//  gpio_num - gpio number 0-31
void gpio_clear(uint8_t gpio_num) {
  gpio->OUTCLR = 1 << gpio_num;
}

// Inputs: 
//  gpio_num - gpio number 0-31
bool gpio_read(uint8_t gpio_num) {
    // should return pin state
    uint32_t state = (gpio->IN >> gpio_num) & 0x1;
    return (bool) state;
}