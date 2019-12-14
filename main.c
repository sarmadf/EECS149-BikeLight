// Button and Switch app
//
// Uses a button and a switch to control LEDs

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#include "app_error.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrfx_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_serial.h"
#include "gpio.h"
#include "mpu9250.h"

#include "buckler.h"

#define BASE 0
#define WAIT 1
#define LEFT 2
#define RIGHT 3 
#define BUFFER_LENGTH 5
#define HALL_SENSOR 4
#define LED 23
#define LOW 0
#define HIGH 1




/*If the hall sensor is near the magnet whose south pole is facing up, */
/*it will return ture, otherwise it will return false.        */
bool isNearMagnet()
{
  int sensorValue = gpio_read(HALL_SENSOR);
  if(sensorValue == LOW)//if the sensor value is LOW?
  {
    return true;//yes,return ture
  }
  else
  {
    return false;//no,return false
  }
  
}

void turnOnLED()
{
  gpio_clear(LED);
}
void turnOffLED()
{
  gpio_set(LED);
}

 
void pinsInit()
{
  gpio_config(24,OUTPUT);
  gpio_config(25,OUTPUT);
  gpio_set(24);
  gpio_set(25);
  gpio_config(HALL_SENSOR, INPUT);
  gpio_config(LED, OUTPUT);
}

void loop() 
{
  if(isNearMagnet())//if the hall sensor is near the magnet?
  {
    turnOnLED();
    nrf_delay_ms(1000);
  }
  else
  {
    turnOffLED();
    nrf_delay_ms(1000);
  }
}

void setup()
{
  pinsInit();
}

void GPIOTE_IRQnHandler(void){
  printf("yeeeeeeee");
  gpio_set(LED);
  gpio_clear(25);
  gpio_clear(24);
}

// LED array
static uint8_t LEDS[3] = {BUCKLER_LED0, BUCKLER_LED1, BUCKLER_LED2};

int buttonIsPressed() {
	return !nrfx_gpiote_in_is_set(BUCKLER_BUTTON0);
}

float getSquaredAngle() {
	return 950;
}


// handler called whenever an input pin changes
void pin_change_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  switch(pin) {
    case BUCKLER_BUTTON0: {
      if (nrfx_gpiote_in_is_set(BUCKLER_BUTTON0)) {
        nrfx_gpiote_out_set(LEDS[0]);
      } else {
        nrfx_gpiote_out_clear(LEDS[0]);
      }
      break;
    }

    case BUCKLER_SWITCH0: {
      if (nrfx_gpiote_in_is_set(BUCKLER_SWITCH0)) {
        nrfx_gpiote_out_set(LEDS[1]);
        nrfx_gpiote_out_clear(LEDS[2]);
      } else {
        nrfx_gpiote_out_clear(LEDS[1]);
        nrfx_gpiote_out_set(LEDS[2]);
      }
      break;
    }
  }
}

int main(void) {
  ret_code_t error_code = NRF_SUCCESS;

  // initialize power management
  error_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  // initialize GPIO driver
  if (!nrfx_gpiote_is_init()) {
    error_code = nrfx_gpiote_init();
  }
  APP_ERROR_CHECK(error_code);

  // NRF_GPIOTE->CONFIG[0] |= 1 << 12;
  // NRF_GPIOTE->CONFIG[0] |= 1 << 11;
  // NRF_GPIOTE->CONFIG[0] |= 1 << 10;
  // NRF_GPIOTE->CONFIG[0] |= 1 << 17; // pg 161
  //  NRF_GPIOTE->CONFIG[0] |= 1;
  // NRF_GPIOTE->INTENSET |= 1;
  // NVIC_EnableIRQ(GPIOTE_IRQn);
  // NVIC_SetPriority(GPIOTE_IRQn,1);
  nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
  nrfx_gpiote_in_init(BUCKLER_GROVE_A0, &in_config, GPIOTE_IRQnHandler);
  nrfx_gpiote_in_event_enable(BUCKLER_GROVE_A0, true);

  // nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
  // in_config.pull = NRF_GPIO_PIN_NOPULL;
  // error_code = nrfx_gpiote_in_init(BUCKLER_BUTTON0, &in_config, pin_change_handler);
  // nrfx_gpiote_in_event_enable(BUCKLER_BUTTON0, true);
  // in_config.pull = NRF_GPIO_PIN_NOPULL;
  // error_code = nrfx_gpiote_in_init(BUCKLER_SWITCH0, &in_config, pin_change_handler);
  // nrfx_gpiote_in_event_enable(BUCKLER_SWITCH0, true);
  // pin_change_handler(0, 0);
  


  // configure leds
  // manually-controlled (simple) output, initially set
  // nrfx_gpiote_out_config_t out_config = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(true);
  // for (int i=0; i<3; i++) {
  //   error_code = nrfx_gpiote_out_init(LEDS[i], &out_config);
  //   APP_ERROR_CHECK(error_code);
  // }

  // configure button and switch
  // input pin, trigger on either edge, low accuracy (allows low-power operation)
  // nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
  // in_config.pull = NRF_GPIO_PIN_NOPULL;
  // error_code = nrfx_gpiote_in_init(BUCKLER_BUTTON0, &in_config, pin_change_handler);
  // nrfx_gpiote_in_event_enable(BUCKLER_BUTTON0, true);

  // in_config.pull = NRF_GPIO_PIN_NOPULL;
  // error_code = nrfx_gpiote_in_init(BUCKLER_SWITCH0, &in_config, pin_change_handler);
  // nrfx_gpiote_in_event_enable(BUCKLER_SWITCH0, true);

  // set initial states for LEDs
  // nrfx_gpiote_out_set(LEDS[0]);
  // if (nrfx_gpiote_in_is_set(BUCKLER_SWITCH0)) {
  //   nrfx_gpiote_out_set(LEDS[1]);
  //   nrfx_gpiote_out_clear(LEDS[2]);
  // } else {
  //   nrfx_gpiote_out_clear(LEDS[1]);
  //   nrfx_gpiote_out_set(LEDS[2]);
  // }

  setup();

  
  // loop forever
  while (1) {
    // loop();
    gpio_set(23);

  }
}



