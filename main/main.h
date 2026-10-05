#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"
#include "led_strip_types.h"
#include "led_strip_rmt.h" 
#include "led_strip.h"
#include <time.h>
#include <sys/time.h>
#include "driver/spi_master.h"
#include <driver/ledc.h>
#include <driver/adc.h>
#include "esp_rom_sys.h"
// #include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"

#include "sensor.h"
#include "led.h"
#include "oled.h"
#include "display.h"
#include "buzzer.h"




#define LED_PIN             GPIO_NUM_8
#define LED_NUM             25


#define DEBOUNCE_TIME_MS    50

// Light mode
#define MODE                3
#define OFF                 0
#define WHITE               1
#define YELLOW              2
#define BLUE                3

// Config SPI for OLED
#define OLED_WIDTH          128
#define OLED_HEIGHT         64

#define OLED_MOSI           GPIO_NUM_6
#define OLED_SCK            GPIO_NUM_4
#define OLED_CS             GPIO_NUM_7
#define OLED_DC             GPIO_NUM_10
#define OLED_RST            GPIO_NUM_3


#define LDR_ADC_CHANNEL     ADC1_CHANNEL_0     // GPIO0
#define DHT22_GPIO          GPIO_NUM_5

// #define OLED_MOSI           GPIO_NUM_2
// #define OLED_SCK            GPIO_NUM_3
// #define OLED_CS             GPIO_NUM_5
// #define OLED_DC             GPIO_NUM_4
// #define OLED_RST            GPIO_NUM_10


// Config BUZZER
#define BUZZER_PIN          GPIO_NUM_18
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL        LEDC_CHANNEL_0
#define LEDC_DUTY_RES       LEDC_TIMER_13_BIT
#define LEDC_DUTY           (4096)
#define LEDC_FREQUENCY      (2000)

#define TEMP_ALARM_ON       350
#define TEMP_ALARM_OFF      340

#endif