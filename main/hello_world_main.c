/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "nvs_flash.h"
#include "mqtt_client.h"

#include <dht.h>
#include <bmp280.h>

static const char *TAG = "MQTT_WEATHER_STATION";

//SENSORS
static const dht_sensor_type_t sensor_type = DHT_TYPE_AM2301;
static const gpio_num_t dht_gpio = 5;
#define SDA_GPIO 16
#define SCL_GPIO 17

//MQTT
#define CONFIG_BROKER_URL "mqtt://192.168.88.159"
#define NO_OF_SAMPLES 10

//WIFI
#define CONFIG_WIFI_SSID "Uai-Fai"
#define CONFIG_WIFI_PASSWORD "lodea12349876"
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

//ADC
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_channel_t solar = ADC1_CHANNEL_7;     //GPIO35 if ADC1
static const adc_channel_t battery = ADC1_CHANNEL_6;     //GPIO34 if ADC1
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate


static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);

            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void wifi_init(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_LOGI(TAG, "start the WIFI SSID:[%s]", CONFIG_WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Waiting for wifi");
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    
}

void sensor(void *pvParamters)
{   
    float pressure, temperature, humidity;
    uint32_t solar_voltage, battery_voltage;
    char buffer[10];
    uint32_t reading;

    //BMP280
    bmp280_params_t params;
    bmp280_init_default_params(&params);
    bmp280_t dev;
    memset(&dev, 0, sizeof(bmp280_t));

    ESP_ERROR_CHECK(bmp280_init_desc(&dev, BMP280_I2C_ADDRESS_0, 0, SDA_GPIO, SCL_GPIO));
    ESP_ERROR_CHECK(bmp280_init(&dev, &params));

    bool bme280p = dev.id == BME280_CHIP_ID;
    printf("BMP280: found %s\n", bme280p ? "BME280" : "BMP280");

    //Configure ADC
    
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(battery, atten);
    adc1_config_channel_atten(solar, atten);

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    
    //MQTT
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);


    while (1)
    {
        vTaskDelay(10000 / portTICK_PERIOD_MS);

        if (bmp280_read_float(&dev, &temperature, &pressure, &humidity) != ESP_OK)
            ESP_LOGE(TAG,"Temperature/pressure reading failed\n");
        
        if (dht_read_float_data(sensor_type, dht_gpio, &humidity, &temperature) != ESP_OK)
            ESP_LOGE(TAG,"Could not read data from DHT\n");
        
        reading = 0;

        esp_adc_cal_get_voltage(solar,adc_chars,&reading); 

        solar_voltage = (reading)*2;
        
        reading = 0;

        esp_adc_cal_get_voltage(battery,adc_chars,&reading); 

        battery_voltage = (reading)*2;
        
        ESP_LOGI(TAG,"Pressure: %.2f Pa, Temperature: %.2f C, Humidity: %.2f\n", pressure, temperature,humidity);
        ESP_LOGI(TAG,"Battery: %dmV, Solar: %dmV\n",battery_voltage,solar_voltage);
        snprintf(buffer, sizeof buffer, "%.2f", temperature);
        esp_mqtt_client_publish(client, "/out/sta0/temperature", buffer, 0, 0, 1);
        snprintf(buffer, sizeof buffer, "%.2f", humidity);
        esp_mqtt_client_publish(client, "/out/sta0/humidity", buffer, 0, 0, 1);
        snprintf(buffer, sizeof buffer, "%.2f", pressure);
        esp_mqtt_client_publish(client, "/out/sta0/pressure", buffer, 0, 0, 1);
        snprintf(buffer, sizeof buffer, "%d", solar_voltage);
        esp_mqtt_client_publish(client, "/out/sta0/solar", buffer, 0, 0, 1);
        snprintf(buffer, sizeof buffer, "%d", battery_voltage);
        esp_mqtt_client_publish(client, "/out/sta0/battery", buffer, 0, 0, 1);


    }
}

void app_main()
{
    printf("Hello world!\n");
    nvs_flash_init();
    wifi_init();
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreate(sensor, "sensor", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);   
}
