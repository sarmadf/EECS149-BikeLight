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

#include "mpu9250.h"

#include "buckler.h"

#define BASE 0
#define WAIT 1
#define LEFT 2
#define RIGHT 3 

#define BUFFER_LENGTH 5

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

  // initialize GPIO driver
  if (!nrfx_gpiote_is_init()) {
    error_code = nrfx_gpiote_init();
  }
  APP_ERROR_CHECK(error_code);

  // configure leds
  // manually-controlled (simple) output, initially set
  nrfx_gpiote_out_config_t out_config = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(true);
  for (int i=0; i<3; i++) {
    error_code = nrfx_gpiote_out_init(LEDS[i], &out_config);
    APP_ERROR_CHECK(error_code);
  }

  // configure button and switch
  // input pin, trigger on either edge, low accuracy (allows low-power operation)
  nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
  in_config.pull = NRF_GPIO_PIN_NOPULL;
  error_code = nrfx_gpiote_in_init(BUCKLER_BUTTON0, &in_config, pin_change_handler);
  nrfx_gpiote_in_event_enable(BUCKLER_BUTTON0, true);

  in_config.pull = NRF_GPIO_PIN_NOPULL;
  error_code = nrfx_gpiote_in_init(BUCKLER_SWITCH0, &in_config, pin_change_handler);
  nrfx_gpiote_in_event_enable(BUCKLER_SWITCH0, true);

  // set initial states for LEDs
  nrfx_gpiote_out_set(LEDS[0]);
  if (nrfx_gpiote_in_is_set(BUCKLER_SWITCH0)) {
    nrfx_gpiote_out_set(LEDS[1]);
    nrfx_gpiote_out_clear(LEDS[2]);
  } else {
    nrfx_gpiote_out_clear(LEDS[1]);
    nrfx_gpiote_out_set(LEDS[2]);
  }

  
  // loop forever
  int state = BASE;
  int buttonBuffer[BUFFER_LENGTH];
  bool isPressed = false;

  for (int i = 0; i < BUFFER_LENGTH; i++){
	buttonBuffer[i] = 0;
  }
  unsigned long long currentTime = 0;
  unsigned long long releaseTime = 0;

  printf("STATE: %d\n", state);

  while (1) {
    bool allOne = true;
    bool allZero = true;
	currentTime++;
    // printf("time: %llu\n", currentTime);

	int buttonState = buttonIsPressed() ? 1 : 0;
	for (int i = 0; i < BUFFER_LENGTH - 1; i++) {
		buttonBuffer[i] = buttonBuffer[i+1];
	}
	buttonBuffer[BUFFER_LENGTH - 1] = buttonState;

	for (int i = 0; i < BUFFER_LENGTH; i++) {
		allOne &= buttonBuffer[i] == 1 ? 0 : 1;
		allZero &= buttonBuffer[i] == 0 ? 0 : 1;
	}

	float angle = getSquaredAngle();

	// printf("current time: %llu", currentTime);
	// printf("release time: %llu", releaseTime);
	 
  	switch(state) {
	  case BASE: 
	    if (!isPressed && allOne) {
			nrf_delay_ms(50);
			isPressed = true;
			state = BASE;
		} else if (isPressed && allZero) {
			nrf_delay_ms(50);
			isPressed = false;

			releaseTime = currentTime;
			state = WAIT;
  printf("STATE: %d\n", state);
		} else {
			state = BASE;
		}

		break;
	  case WAIT: 
	  	if (currentTime - releaseTime > 5000000) {
		    state = LEFT;
  printf("STATE: %d\n", state);
		} else if (!isPressed && allOne) {
			printf("pressed ACTIVATED\n");
			nrf_delay_ms(50);
			isPressed = true;
			state = WAIT;
		} else if (isPressed && allZero) {
			printf("unpressed ACTIVATED\n");
			nrf_delay_ms(50);
			isPressed = false;
			state = RIGHT;
  printf("STATE: %d\n", state);
		}

		break;

	  case LEFT: 
	  	if (angle < 900) state = BASE;
  printf("STATE: %d\n", state);
		break;
	  case RIGHT: 
	  	if (angle < 900) state = BASE;
  printf("STATE: %d\n", state);
		break;
	}
  }
}



