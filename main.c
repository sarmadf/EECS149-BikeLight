// Drive the Kobuki robot in a square using the internal gyroscope

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
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

#include "buckler.h"
#include "kobukiActuator.h"
#include "kobukiSensorTypes.h"
#include "kobukiSensorPoll.h"
#include "kobukiUtilities.h"

typedef enum {
  NONE,
  LEFT,
  RIGHT,
  DELAY_L,
  DELAY_R,
  WAIT
} TurningState_t;

#define BUCKLER_BUTTON0 NRF_GPIO_PIN_MAP(0,28)

bool getButtonState(){
	return nrfx_gpiote_in_is_set(BUCKLER_BUTTON0);
}

int main(void) {

  // initialize Kobuki library
//  kobukiInit();

  // initialize RTT library
  NRF_LOG_INIT(NULL);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Initialized RTT!\n");

  // initialize state
  TurningState_t state = NONE;
//  KobukiSensors_t initial_sensors;
//  kobukiSensorPoll(&initial_sensors);
  ret_code_t error_code = NRF_SUCCESS;

  // initialize power management
  error_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(error_code);

  // initialize GPIO driver
  if (!nrfx_gpiote_is_init()) {
    error_code = nrfx_gpiote_init();
  }
  APP_ERROR_CHECK(error_code);

  nrfx_gpiote_out_config_t out_config = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(true);
  
  nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
  in_config.pull = NRF_GPIO_PIN_NOPULL;
  // error_code = nrfx_gpiote_in_init(BUCKLER_BUTTON0, &in_config, pin_change_handler);
  nrfx_gpiote_in_event_enable(BUCKLER_BUTTON0, true);

  // loop forever
  uint8_t i = 0;
  while (1) {
  	bool buttonState = getButtonState();
	printf("%d%d%d%d%d%d\n", buttonState, buttonState, buttonState, buttonState, buttonState, buttonState);
    // test current state
    switch (state) {
      case NONE: 
	  	if (true) {
			state = NONE;
        } else {
			state = NONE;
		}

        break;	
    };

    // continue for 10 ms before checking state again
    nrf_delay_ms(10);
	printf("OOJOJOJADSFASLDFJ\n");
  }
}


