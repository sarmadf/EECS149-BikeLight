// Button and Switch app
//
// Uses a button and a switch to control LEDs

#include <stdbool.h>
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

#define HALL_SENSOR 4
#define LED 23
#define LOW 0
#define HIGH 1
#define OFF 0
#define ON 1


int rpm_count = 0; // keeps track of number of revolutions 
int time_start = 0; //keeps track of time after hall sensor senses the first revolution
int rpm = 0; //revolutions per minute
int mph = 0; //keep track of current speed in MPH
int rotating_time = 0; // time taken for rotatation to complete
bool timer_started = false;


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
 

int main(void){
  ret_code_t error_code = NRF_SUCCESS;

  // initialize power management
  error_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();

  // initialize GPIO driver
  if (!nrfx_gpiote_is_init()){
    error_code = nrfx_gpiote_init();
  }
  APP_ERROR_CHECK(error_code);

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

  nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
  nrfx_gpiote_in_init(BUCKLER_GROVE_A0, &in_config, Hall_SensorHandler);
  nrfx_gpiote_in_event_enable(BUCKLER_GROVE_A0, true);
  virtual_timer_init();
  uint32_t timer0_on = virtual_timer_start_repeated(1000000, led0_toggle);
  
  //setup on board LED as temporary placeholder for led
  gpio_config(LED,OUTPUT);
  
  display_write("speed mph", DISPLAY_LINE_0);
  int backlight_state = OFF;
  int previous_speed = 0;
  int acceleration = 0;
  // loop forever
  while (1) {
    previous_speed = mph;
    update_speed();
    long range = MeasureInInches();
    printf("%lu \n",range);
    
    if (is_slowing_down(previous_speed,mph)){
      backlight_state = ON;
      printf("is slowing down\n");
    }
    else{
      backlight_state = OFF;
      // printf("speed constant or speeding up\n");
    }
    
    if (backlight_state){
      turnOnLED();
      nrf_delay_ms(1000);
    }
    else{
      turnOffLED();
    }
  }
}



