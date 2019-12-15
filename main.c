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
#include "display.h"
#include "buckler.h"
#include "virtual_timer.h"
#define BASE 0
#define WAIT 1
#define LEFT 2
#define RIGHT 3 
#define BUFFER_LENGTH 5
#define HALL_SENSOR 4
#define LED 23
#define LOW 0
#define HIGH 1


int rpm_count = 0;
int time_old = 0;
int rpm = 0;//revolutions per minute
int rotating_time = 0;

// uint32_t millis(void)
// {
//   return(app_timer_cnt_get() / 32.768);
// }

// #define OVERFLOW ((uint32_t)(0xFFFFFFFF/32.768))

// uint32_t compareMillis(uint32_t previousMillis, uint32_t currentMillis)
// {
//   if(currentMillis < previousMillis) return(currentMillis + OVERFLOW + 1 - previousMillis);
//   return(currentMillis - previousMillis);
// }
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
  // if(isNearMagnet())//if the hall sensor is near the magnet?
  // {
  //   turnOnLED();
  //   nrf_delay_ms(1000);
  // }
  // else
  // {
  //   turnOffLED();
  //   nrf_delay_ms(1000);
  // }
  
  if (rpm_count == 1) {
		time_old = read_timer();
    // printf("time=%d\n", time_old);
  }
	else if (rpm_count > 20) 
	{ 
    printf("timer old:%d\n",time_old);
    printf("current time %d\n", read_timer());
		rotating_time = (read_timer() - time_old) / 1000;
		printf("rotating timer:%d\n",rotating_time);
    //Update RPM every 20 counts, increase this for better RPM resolution,
		//decrease for faster update
		rpm = 60000/rotating_time*(rpm_count-1);//1 min = 60000ms
		rpm_count = 0;
    char buf[32] = {0};
    snprintf(buf, 32, "%u", rpm);
    // printf()
    display_write(buf, rpm);
    printf("rpm:%d\n",rpm);
    int bike_wheel = 151;
    int mph = (151 * rpm * 60) / 63360;
    printf("mph: %d", mph);
		// Serial.println(rpm, DEC);
	}
}
 


// void rpm_fun()
// {
// 	rpmcount++;
// }

void setup()
{
  pinsInit();
}

void GPIOTE_IRQnHandler(void){
  // printf("yeeeeeeee");
  gpio_set(LED);
  // gpio_clear(25);
  // gpio_clear(24);
  // void rpm_fun()

	rpm_count++;
  printf("rpm count:%d\n",rpm_count);

}

void led0_toggle() {
  //  loop();
  
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
  virtual_timer_init();
  uint32_t timer0_on = virtual_timer_start_repeated(1000, led0_toggle);
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
    loop();
    // gpio_set(23);
    
    // nrf_delay_ms(1000);

  }
}



