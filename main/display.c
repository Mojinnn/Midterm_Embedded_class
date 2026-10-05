#include "display.h"
#include "buzzer.h"

static const char *TAG = "DISPLAY";


// Initialization
void display_init(void) {
    oled_init();
    oled_clear();
    ESP_LOGI(TAG, "Display initialized");
}