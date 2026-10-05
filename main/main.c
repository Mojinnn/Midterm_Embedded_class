#include "main.h"

// Data Structure

QueueHandle_t xQueue;
QueueHandle_t xBuzzerQueue;

typedef enum
{
    temp,
    light
} DataSource_t;

typedef struct
{
    uint32_t ucValue;
    DataSource_t eDataSource;
} Data_t;

// Support Function
void display_update(const Data_t *data)
{
    oled_clear();

    int time_col = 20;
    int temp_int = data->ucValue / 10;
    int temp_decimal = (data->ucValue % 10);

    oled_draw_large_char(3, time_col, (temp_int / 10) + '0');
    oled_draw_large_char(3, time_col + 16, (temp_int % 10) + '0');
    oled_draw_large_char(3, time_col + 32, '.');
    oled_draw_large_char(3, time_col + 46, temp_decimal + '0');
    oled_draw_large_char(3, time_col + 60, 'C');
}

static void lightAdjust(const Data_t *data)
{
    static int smoothed = -1;
    static int last_brightness = -1;

    int v = (data->ucValue > 4095) ? 4095 : (int)data->ucValue;
    smoothed = (smoothed < 0) ? v : (smoothed * 3 + v) / 4;
    int percent = (smoothed * 100) / 4095;
    if (percent < 20) percent = 0;

    if (last_brightness < 0 || abs(percent - last_brightness) >= 3) {
        led_set_brightness(percent);
        last_brightness = percent;
    }
}

static void checkTempAlarm(uint32_t temperature)
{
    static bool alarm_active = true;
    uint8_t cmd = 1;

    if (alarm_active && temperature > TEMP_ALARM_ON) {
        alarm_active = false;
        xQueueSendToBack(xBuzzerQueue, &cmd, 0);
        printf("ALARM: temp too high!\r\n");
    }
    else {
        alarm_active = true;
    }
}

static void buzzerTask(void *pvParameters)
{
    uint8_t cmd;

    for (;;)
    {
        if (xQueueReceive(xBuzzerQueue, &cmd, portMAX_DELAY) == pdPASS)
        {
            buzzer_beep_5s();
        }
    }
}

// Task

static void tempSenderTask( void *pvParameters )
{
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS( 100 );

    for( ;; )
    {
        uint32_t tempValue = readTemp();

        Data_t tempData = {
            .eDataSource = temp,
            .ucValue = tempValue
        };

        xStatus = xQueueSendToBack( xQueue, &tempData, xTicksToWait );

        if( xStatus != pdPASS )
        {
            printf( "Could not send to the queue.\r\n" );
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void lightSenderTask( void *pvParameters )
{
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS( 100 );

    for( ;; )
    {
        uint32_t ligthValue = readLight();

        Data_t lightData = {
            .eDataSource = light,
            .ucValue = ligthValue
        };

        xStatus = xQueueSendToBack( xQueue, &lightData, xTicksToWait );

        if( xStatus != pdPASS )
        {
            printf( "Could not send to the queue.\r\n" );
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void vReceiverTask( void *pvParameters )
{
    Data_t xReceivedStructure;
    BaseType_t xStatus;

    for( ;; )
    {

        if( uxQueueMessagesWaiting( xQueue ) == 3 )
        {
            printf( "Queue is full!\r\n" );
        }

        xStatus = xQueueReceive(xQueue, &xReceivedStructure, portMAX_DELAY);

        if( xStatus == pdPASS )
        {
            if( xReceivedStructure.eDataSource == temp )
            {
                printf( "Temp = %" PRIu32 "\n", xReceivedStructure.ucValue );
                display_update(&xReceivedStructure);
                checkTempAlarm(xReceivedStructure.ucValue);
            }
            else if ( xReceivedStructure.eDataSource == light )
            {
                printf( "Light = %" PRIu32 "\n", xReceivedStructure.ucValue );
                lightAdjust(&xReceivedStructure);
            }
        }
        else
        {
            printf( "Could not receive from the queue.\r\n" );
        }
    }
}

void app_main ()
{
    xQueue = xQueueCreate( 3, sizeof( Data_t ) );
    xBuzzerQueue = xQueueCreate(1, sizeof(uint8_t));

    ldr_init();
    dht22_init();
    oled_init();
    led_init();
    buzzer_init();

    if( xQueue != NULL )
    {
        xTaskCreate( buzzerTask, "Buzzer", 2048, NULL, 3, NULL );

        xTaskCreate( tempSenderTask, "TempSender", 4096, NULL, 2, NULL );
        xTaskCreate( lightSenderTask, "LightSender", 4096, NULL, 2, NULL );


        xTaskCreate( vReceiverTask, "Receiver", 4096, NULL, 1, NULL );
    }
    else
    {
        /* The queue could not be created. */
    }
}