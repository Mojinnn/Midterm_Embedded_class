#include "sensor.h"
#include "dht.h"

void ldr_init(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(LDR_ADC_CHANNEL, ADC_ATTEN_DB_11);
}

uint32_t readLight(void)
{
    uint32_t sum = 0;
    for (int i = 0; i < 16; i++) 
        sum += adc1_get_raw(LDR_ADC_CHANNEL);
    return sum / 16;
}

// static bool wait_level(int level, uint32_t timeout_us)
// {
//     int64_t start = esp_timer_get_time();

//     while (gpio_get_level(DHT22_GPIO) != level) {
//         if ((esp_timer_get_time() - start) > timeout_us) return false;
//     }

//     return true;
// }

void dht22_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DHT22_GPIO),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    gpio_set_level(DHT22_GPIO, 1);
}

static portMUX_TYPE dht_mux = portMUX_INITIALIZER_UNLOCKED;

static bool wait_level(int level, uint32_t timeout_us)
{
    int64_t start = esp_timer_get_time();
    while (gpio_get_level(DHT22_GPIO) != level) {
        if ((esp_timer_get_time() - start) > timeout_us) return false;
    }
    return true;
}

static bool dht_read_raw(uint8_t data[5])
{
    uint32_t high_us[40] = {0};
    bool ok = true;

    gpio_set_level(DHT22_GPIO, 0);
    esp_rom_delay_us(20000);          // >= 18ms
    gpio_set_level(DHT22_GPIO, 1);
    esp_rom_delay_us(30);

    portENTER_CRITICAL(&dht_mux);

    if (!wait_level(0, 200) || !wait_level(1, 200) || !wait_level(0, 200)) {
        ok = false;
    }

    for (int bit = 0; ok && bit < 40; bit++) {
        if (!wait_level(1, 120)) { ok = false; break; }
        int64_t start = esp_timer_get_time();
        if (!wait_level(0, 150)) { ok = false; break; }
        high_us[bit] = (uint32_t)(esp_timer_get_time() - start);
    }

    portEXIT_CRITICAL(&dht_mux);

    if (!ok) return false;

    memset(data, 0, 5);
    for (int bit = 0; bit < 40; bit++) {
        data[bit / 8] <<= 1;
        if (high_us[bit] > 45) data[bit / 8] |= 1;
    }
    return true;
}

uint32_t readTemp(void)
{
    static uint32_t last_temp = 0;
    uint8_t data[5];

    if (!dht_read_raw(data)) {
        printf("DHT: timeout\n");
        return last_temp;
    }

    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        printf("DHT: checksum fail %d %d %d %d %d\n",
               data[0], data[1], data[2], data[3], data[4]);
        return last_temp;
    }
    last_temp = data[2] * 10 + data[3];

    return last_temp;
}