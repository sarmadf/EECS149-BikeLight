// Button and Switch app
//
// Uses a button and a switch to control LEDs

// #include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
// #include <math.h>
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
#include "display.h"
#include "buckler.h"
#include "virtual_timer.h"
#include "Ultrasonic.h"


#define BASE 0
#define WAIT 1
#define LEFT 2
#define RIGHT 3

#define BUFFER_LENGTH 100 
#define TURN_TIME 100

#define RF_INPUT_PIN NRF_GPIO_PIN_MAP(0,28)
#define LEFT_PIN NRF_GPIO_PIN_MAP(0,25)
#define RIGHT_PIN NRF_GPIO_PIN_MAP(0,23)
#define BRAKE_PIN NRF_GPIO_PIN_MAP(0,24)
// #define SPEAKER_PIN NRF_GPIO_PIN_MAP(0,30)

#define LED 23
#define BUFFER_LENGTH 5
#define OFF 0
#define ON 1

int rpm_count = 0; // keeps track of number of revolutions 
int time_start = 0; //keeps track of time after hall sensor senses the first revolution
int rpm = 0; //revolutions per minute
int mph = 0; //keep track of current speed in MPH
int rotating_time = 0; // time taken for rotatation to complete
bool timer_started = false; // makes sure timer start only gets called once 

NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);
// LED array
static uint8_t LEDS[3] = {BUCKLER_LED0, BUCKLER_LED1, BUCKLER_LED2};

int buttonIsPressed() {
	// return !nrfx_gpiote_in_is_set(BUCKLER_BUTTON0);
	return nrfx_gpiote_in_is_set(RF_INPUT_PIN);
}

float getAccel(mpu9250_measurement_t accel) {
	return accel.x_axis * accel.x_axis + accel.y_axis * accel.y_axis; 
}

bool isBraking() {
	return getAccel(mpu9250_read_accelerometer()) > 0.5;
}

float getAngle(mpu9250_measurement_t accel) {
  float theta = atan(accel.y_axis / sqrt((accel.x_axis * accel.x_axis) + (accel.z_axis * accel.z_axis)));
  //printf("theta: %f\n", theta * 180 / 3.14159265358979);

  return theta * 180 / 3.1415926538979;
}

// handler called whenever an input pin changes
void pin_change_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  switch(pin) {
    case BUCKLER_BUTTON0: {
      if (nrfx_gpiote_in_is_set(BUCKLER_BUTTON0)) {
        // nrfx_gpiote_out_set(LEDS[0]);
      } else {
        // nrfx_gpiote_out_clear(LEDS[0]);
      }
      break;
    }

    case BUCKLER_SWITCH0: {
      if (nrfx_gpiote_in_is_set(BUCKLER_SWITCH0)) {
        // nrfx_gpiote_out_set(LEDS[1]);
        // nrfx_gpiote_out_clear(LEDS[2]);
      } else {
        // nrfx_gpiote_out_clear(LEDS[1]);
        // nrfx_gpiote_out_set(LEDS[2]);
      }
      break;
    }
  }
}

//interrupt handler for whenever hall sensor senses magnet come around
void Hall_SensorHandler(void){
	rpm_count++;
}

// timer interrup handler
void led0_toggle() {
}

// check if the bike is slowing down
bool is_slowing_down(int previous_speed, int new_speed){
  int acceleration = new_speed - previous_speed;
    // system slowing down if old speed higer than new speed
    if (acceleration < 0){
      return true;
    }
    else{
      return false;
    }
}

void turnOnLED(){
  gpio_clear(LED);
}
void turnOffLED(){
  gpio_set(LED);
}

int update_speed(){

  // TODO: add conditon to make sure that insane rpm cannot be true
  if (rpm_count == 1 && timer_started == false) {
    printf("rpm count = 1\n");
		time_start = read_timer();
    timer_started = true;
  }
	else if (rpm_count >= 2){
    //higher rpm_count will result in a better reading of speed
    printf("rpm count >= 2\n");
    printf("timer start: %d\n", time_start);
		rotating_time = (read_timer() - time_start) / 1000;
    printf("rotating_time: %d\n",rotating_time);
		rpm = 60000/rotating_time*(rpm_count-1);//1 min = 60000ms
    printf("rpm: %d\n",rpm);
		rpm_count = 0;
    int bike_wheel = 151; // 151 not correct, this should be circumference of bike wheel in inches
    mph = (84.842 * rpm * 60) / 63360; // first part calculates inches per hour and second part is division 12x5280 for mph
    printf("mph: %d\n",mph);
    timer_started = false;
	}
  int zero_mph_threshold = read_timer() - time_start;
  /* if 2 seconds have passed without any update in rpm then reset and mph is 0
    (confirm how long it takes for 2 revolutions at 1 mph and set to below that time, arbirarily set to 10 seconds for now )
    should be 12.5 rpm which means .207 rps and for one revolution to complete it will take 4.9s */
    //TODO: confirm this is 10s
  if (zero_mph_threshold > 10000000 && rpm_count <= 1 && time_start > 0){
    printf("zero_mph_threshold\n");
    rpm_count = 0;
    mph = 0;
    timer_started = false;
  }
  char buf[32] = {0};
  snprintf(buf, 32, "%u", mph);
  display_write(buf, DISPLAY_LINE_1);
  return mph;
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

  // initialize i2c master
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl = BUCKLER_SENSORS_SCL;
  i2c_config.sda = BUCKLER_SENSORS_SDA;
  i2c_config.frequency = NRF_TWIM_FREQ_100K;
  error_code = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
  APP_ERROR_CHECK(error_code);
  mpu9250_init(&twi_mngr_instance);
  printf("IMU initialized!\n");

   // initialize spi master
  nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
  nrf_drv_spi_config_t spi_config = {
    .sck_pin = BUCKLER_LCD_SCLK,
    .mosi_pin = BUCKLER_LCD_MOSI,
    .miso_pin = BUCKLER_LCD_MISO,
    .ss_pin = BUCKLER_LCD_CS,
    .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
    .orc = 0,
    .frequency = NRF_DRV_SPI_FREQ_4M,
    .mode = NRF_DRV_SPI_MODE_2,
    .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
  };
  error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
  APP_ERROR_CHECK(error_code);

  // initialize display driver
  display_init(&spi_instance);
  printf("Display initialized\n");
  nrf_delay_ms(1000);

  // configure leds
  // manually-controlled (simple) output, initially set
  nrfx_gpiote_out_config_t out_config = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(true);
  for (int i=0; i<3; i++) {
     error_code = nrfx_gpiote_out_init(LEDS[i], &out_config);
	 APP_ERROR_CHECK(error_code);
  }

  //nrfx_gpiote_out_init(TEST_LED, &out_config);

  // configure button and switch
  // input pin, trigger on either edge, low accuracy (allows low-power operation)
  nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
  in_config.pull = NRF_GPIO_PIN_NOPULL;
  error_code = nrfx_gpiote_in_init(BUCKLER_BUTTON0, &in_config, pin_change_handler);
  nrfx_gpiote_in_event_enable(BUCKLER_BUTTON0, true);

  in_config.pull = NRF_GPIO_PIN_NOPULL;
  error_code = nrfx_gpiote_in_init(BUCKLER_SWITCH0, &in_config, pin_change_handler);
  nrfx_gpiote_in_event_enable(BUCKLER_SWITCH0, true);

  in_config.pull = NRF_GPIO_PIN_NOPULL;
  error_code = nrfx_gpiote_in_init(RF_INPUT_PIN, &in_config, pin_change_handler);
  nrfx_gpiote_in_event_enable(RF_INPUT_PIN, true);

  // attempt to configure an external button
  nrfx_gpiote_in_config_t in_config_2 = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
  in_config_2.pull = NRF_GPIO_PIN_NOPULL;
  error_code = nrfx_gpiote_in_init(RF_INPUT_PIN, &in_config_2, pin_change_handler);
  nrfx_gpiote_in_event_enable(RF_INPUT_PIN, true);

  // set initial states for LEDs
  //nrfx_gpiote_out_clear(LEDS[0]);
  //nrfx_gpiote_out_clear(LEDS[1]);
  //nrfx_gpiote_out_clear(LEDS[2]);
  if (nrfx_gpiote_in_is_set(BUCKLER_SWITCH0)) {
    //nrfx_gpiote_out_set(LEDS[1]);
    //nrfx_gpiote_out_clear(LEDS[2]);
  } else {
    //nrfx_gpiote_out_clear(LEDS[1]);
    //nrfx_gpiote_out_set(LEDS[2]);
  }

  // initialize FSM 
  int state = BASE;
  int buttonBuffer[BUFFER_LENGTH];
  bool isPressed = false;

  for (int i = 0; i < BUFFER_LENGTH; i++){
	buttonBuffer[i] = 0;
  }
  unsigned long long currentTime = 0;
  unsigned long long releaseTime = 0;
  unsigned long long turnStartTime = 0;
  bool inTurn = false;

  printf("STATE: %d\n", state);


  nrfx_gpiote_in_config_t grove_config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
  nrfx_gpiote_in_init(BUCKLER_GROVE_A0, &grove_config, Hall_SensorHandler);
  nrfx_gpiote_in_event_enable(BUCKLER_GROVE_A0, true);
  virtual_timer_init();
  uint32_t timer0_on = virtual_timer_start_repeated(1000000, led0_toggle);
  
  //setup on board LED as temporary placeholder for led
  gpio_config(LED,OUTPUT);
  gpio_config(9,OUTPUT);
  display_write("speed mph", DISPLAY_LINE_0);

  int backlight_state = OFF;
  int previous_speed = 0;
  int acceleration = 0;


  // loop forever
  while (1) {
	long range = MeasureInInches();
    printf("%lu \n",range);
    previous_speed = mph;
    update_speed();
    
    if (range < 50) {
      gpio_clear(9);
    } else
    {
      gpio_set(9);
    }
    


    if (is_slowing_down(previous_speed,mph)){
      backlight_state = ON;
      printf("is slowing down\n");
    }
    else{
      backlight_state = OFF;
      // printf("speed constant or speeding up\n");
    }
    
    if (backlight_state){
      // turnOnLED();
      // nrf_delay_ms(1000);
    }
    else{
      // turnOffLED();
    }
	
	nrf_delay_ms(1);
	uint32_t rfButton = nrfx_gpiote_in_is_set(RF_INPUT_PIN);
	//printf("count: %llu\n", currentTime);
    bool allOne = true;
    bool allZero = true;
	currentTime++;
    // printf("time: %llu\n", currentTime);

	int buttonState = buttonIsPressed() ? 0 : 1;
	// printf("rf button state: %d\n", buttonState);
	for (int i = 0; i < BUFFER_LENGTH - 1; i++) {
		buttonBuffer[i] = buttonBuffer[i+1];
	}
	buttonBuffer[BUFFER_LENGTH - 1] = buttonState;

	for (int i = 0; i < BUFFER_LENGTH; i++) {
		allOne &= buttonBuffer[i] == 1;
		allZero &= buttonBuffer[i] == 0;
	}

	if (isBraking()) {
		nrfx_gpiote_out_set(BRAKE_PIN);
	} else {
		nrfx_gpiote_out_clear(BRAKE_PIN);
	}
/*
	if ((currentTime / 1000) % 2 == 0) {
		nrfx_gpiote_out_set(LEFT_PIN);
		//nrfx_gpiote_out_clear(RIGHT_PIN);
	} else {
       nrfx_gpiote_out_clear(LEFT_PIN);
	   //nrfx_gpiote_out_set(RIGHT_PIN); 
	}
*/	
  	switch(state) {
	  case BASE:
		nrfx_gpiote_out_clear(LEFT_PIN);
		nrfx_gpiote_out_clear(RIGHT_PIN);

	    if (!isPressed && allOne) {
			nrf_delay_ms(50);
			isPressed = true;
			state = BASE;
		} else if (isPressed && allZero) {
			nrf_delay_ms(250);
			isPressed = false;

			releaseTime = currentTime;
			state = WAIT;
  			printf("STATE: %d\n", state);
		} else {
			state = BASE;
		}

		break;

	  case WAIT:
	  	if (currentTime - releaseTime > 1000 && !isPressed && allZero) {
			nrf_delay_ms(250);
			isPressed = false;
			turnStartTime = currentTime;
			state = LEFT;
			printf("STATE: %d\n", state);
		} else if (!isPressed && allOne) {
			printf("pressed ACTIVATED\n");
			nrf_delay_ms(50);
			isPressed = true;
			state = WAIT;
		} else if (isPressed && allZero) {
			printf("unpressed ACTIVATED\n");
			nrf_delay_ms(250);
			isPressed = false;
			turnStartTime = currentTime;
			state = RIGHT;
			printf("STATE: %d\n", state);
		}

		break;

		

	  case LEFT:
	  case RIGHT:
	  	//mpu9250_measurement_t accel = mpu9250_read_accelerometer();
	  	//float theta = atan(accel.y_axis / sqrt((accel.x_axis * accel.x_axis) + (accel.z_axis * accel.z_axis)));
	  	//float angle = getAngle(mpu9250_read_accelerometer());
		//printf("angle:%f",abs(theta));
		// while (abs(getAngle(mpu9250_read_accelerometer())) < 30) {
		// printf("p:%d z:%d\n", isPressed, allZero);
		// 	if (buttonState && allZero){
		// 		printf("p:%d z:%d\n", isPressed, allZero);
		//
		// 		nrf_delay_ms(50);
		// 		break;
		// 		}
		if (abs(getAngle(mpu9250_read_accelerometer())) > 40) {
			inTurn = true;
		}

		if (((currentTime - turnStartTime) / TURN_TIME) % 2 == 0) {
			if (state == LEFT) {
				nrfx_gpiote_out_set(LEFT_PIN);
			} else if (state == RIGHT){
				nrfx_gpiote_out_set(RIGHT_PIN);
			}
	    } else {
    		if (state == LEFT) {
	    		nrfx_gpiote_out_clear(LEFT_PIN);
	    	} else if (state == RIGHT){
	    		nrfx_gpiote_out_clear(RIGHT_PIN);
	    	}
		}

		if (inTurn && abs(getAngle(mpu9250_read_accelerometer())) < 30){
			nrf_delay_ms(50);
			inTurn = false;
			state = BASE;
			printf("STATE: %d\n", state);
		} else if (!isPressed && allOne) {
			printf("pressed ACTIVATED\n");
			nrf_delay_ms(50);
			isPressed = true;
		} else if (isPressed && allZero) {
			printf("unpressed ACTIVATED\n");
			nrf_delay_ms(250);
			isPressed = false;
			state = BASE;
			printf("STATE: %d\n", state);
		} 
		// nrfx_gpiote_out_clear(LEFT_PIN);
		// nrfx_gpiote_out_clear(RIGHT_PIN);
		break;
	}
  }
}
