#include "APP_freeRTOS.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"
#include "adc.h"
#include "dht11.h"
#include "socket.h"
#include "stm32f10x.h"
#include <stdio.h>

/* ---------- 事件组 ---------- */
static EventGroupHandle_t xEventGroup = NULL;
#define BIT_ADC_DONE (1 << 0)
#define BIT_DHT_DONE (1 << 1)

/* ---------- 互斥量 ---------- */
static SemaphoreHandle_t xSerialMutex = NULL;

/* ---------- 全局数据 ---------- */
static float g_mcu_temp = 0.0f;
static float g_light_v = 0.0f;
static float g_vref = 0.0f;

float dht_temperature = 0.0f;
float dht_humidity = 0.0f;
uint8_t dht_valid = 0;

TaskHandle_t xADCTaskHandle = NULL;
static TaskHandle_t xUploadTaskHandle = NULL;

#define FLASK_IP {192, 168, 1, 10}
#define FLASK_PORT 5000
#define UPLOAD_SOCKET 7

static void upload_to_flask(float mcu_temp, float light_v, float vref,
                            float dht_temp, float dht_humi)
{
    uint8_t flask_ip[4] = FLASK_IP;
    char json_body[256];
    char http_request[512];
    int body_len, request_len;

    body_len = snprintf(json_body, sizeof(json_body),
                        "{"
                        "\"mcu_temp\":%.2f,"
                        "\"light_v\":%.3f,"
                        "\"vref\":%.3f,"
                        "\"dht_temp\":%.1f,"
                        "\"dht_humi\":%.1f"
                        "}",
                        mcu_temp, light_v, vref,
                        dht_valid ? dht_temp : 0.0f,
                        dht_valid ? dht_humi : 0.0f);

    request_len = snprintf(http_request, sizeof(http_request),
                           "POST /upload HTTP/1.1\r\n"
                           "Host: %d.%d.%d.%d:%d\r\n"
                           "Content-Type: application/json\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           flask_ip[0], flask_ip[1], flask_ip[2], flask_ip[3], FLASK_PORT,
                           body_len, json_body);

    if (socket(UPLOAD_SOCKET, Sn_MR_TCP, 0, 0) != UPLOAD_SOCKET)
    {
        xSemaphoreTake(xSerialMutex, portMAX_DELAY);
        printf("Upload: socket create failed\r\n");
        xSemaphoreGive(xSerialMutex);
        return;
    }

    if (connect(UPLOAD_SOCKET, flask_ip, FLASK_PORT) != SOCK_OK)
    {
        static uint8_t first_fail = 1;
        if (first_fail)
        {
            xSemaphoreTake(xSerialMutex, portMAX_DELAY);
            printf("Upload: connect failed (Flask not running?)\r\n");
            xSemaphoreGive(xSerialMutex);
            first_fail = 0;
        }
        close(UPLOAD_SOCKET);
        return;
    }

    if (send(UPLOAD_SOCKET, (uint8_t *)http_request, request_len) != request_len)
    {
        xSemaphoreTake(xSerialMutex, portMAX_DELAY);
        printf("Upload: send failed\r\n");
        xSemaphoreGive(xSerialMutex);
    }
    else
    {
        xSemaphoreTake(xSerialMutex, portMAX_DELAY);
        printf("Upload: data sent OK\r\n");
        xSemaphoreGive(xSerialMutex);
    }

    disconnect(UPLOAD_SOCKET);
    close(UPLOAD_SOCKET);
}

static void vTask_ADC(void *pvParameters)
{

    for (;;)
    {
        while (adc_data_ready == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        adc_data_ready = 0;

        float temp = Get_Internal_Temperature();
        float light = Get_Light_Voltage();
        float vref = Get_ADC_Reference_Voltage();

        g_mcu_temp = temp;
        g_light_v = light;
        g_vref = vref;

        xEventGroupSetBits(xEventGroup, BIT_ADC_DONE);

        xSemaphoreTake(xSerialMutex, portMAX_DELAY);
        printf("[ADC] Temp=%.2f, Light=%.3f, Vref=%.3f\r\n", temp, light, vref);
        xSemaphoreGive(xSerialMutex);
    }
}

static void vTask_DHT(void *pvParameters)
{
    float temp, humi;
    uint8_t flag_ok;

    for (;;)
    {
        taskENTER_CRITICAL();
        flag_ok = DHT11_Read(&temp, &humi);
        taskEXIT_CRITICAL();

        if (flag_ok)
        {
            dht_temperature = temp;
            dht_humidity = humi;
            dht_valid = 1;
        }
        else
        {
            dht_valid = 0;
        }

        xEventGroupSetBits(xEventGroup, BIT_DHT_DONE);

        xSemaphoreTake(xSerialMutex, portMAX_DELAY);
        if (flag_ok)
            printf("[DHT] Temp=%.1f, Humi=%.1f%%\r\n", temp, humi);
        else
            printf("[DHT] Read error\r\n");
        xSemaphoreGive(xSerialMutex);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void vTask_DataProcessor(void *pvParameters)
{
    EventBits_t bits;

    for (;;)
    {
        bits = xEventGroupWaitBits(xEventGroup,
                                   BIT_ADC_DONE | BIT_DHT_DONE,
                                   pdTRUE, pdTRUE, portMAX_DELAY);

        if ((bits & (BIT_ADC_DONE | BIT_DHT_DONE)) == (BIT_ADC_DONE | BIT_DHT_DONE))
        {
            xSemaphoreTake(xSerialMutex, portMAX_DELAY);
            printf("--- Data Frame ---\r\n");
            printf(" MCU Temp: %.2f C\r\n", g_mcu_temp);
            printf(" Light: %.3f V\r\n", g_light_v);
            printf(" VREF: %.3f V\r\n", g_vref);
            if (dht_valid)
            {
                printf(" DHT Temp: %.1f C\r\n", dht_temperature);
                printf(" DHT Humi: %.1f %%\r\n", dht_humidity);
            }
            else
            {
                printf(" DHT: Read Error\r\n");
            }
            printf("-----------------\r\n");
            xSemaphoreGive(xSerialMutex);

            if (xUploadTaskHandle != NULL)
                xTaskNotifyGive(xUploadTaskHandle);
        }
    }
}

static void vTask_Upload(void *pvParameters)
{
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        upload_to_flask(g_mcu_temp, g_light_v, g_vref, dht_temperature, dht_humidity);
    }
}

void APP_FreeRTOS_Init(void)
{
    xEventGroup = xEventGroupCreate();
    if (xEventGroup == NULL)
    {
        printf("Event group failed!\r\n");
        while (1)
            ;
    }

    xSerialMutex = xSemaphoreCreateMutex();
    if (xSerialMutex == NULL)
    {
        printf("Serial mutex failed!\r\n");
        while (1)
            ;
    }

    xTaskCreate(vTask_ADC, "ADC_Task", 128, NULL, 3, &xADCTaskHandle);
    xTaskCreate(vTask_DHT, "DHT_Task", 200, NULL, 3, NULL);
    xTaskCreate(vTask_DataProcessor, "Data_Task", 256, NULL, 2, NULL);
    xTaskCreate(vTask_Upload, "Upload_Task", 512, NULL, 2, &xUploadTaskHandle);

    vTaskStartScheduler();
    printf("Scheduler start failed!\r\n");
    while (1)
        ;
}