#ifndef _GATEWAY_H_
#define _GATEWAY_H_


#include "stm32g4xx_hal.h"
#include "common.h"
#include "cJSON.h"

// 引脚定义
#define LORA_AUX_GPIO_PORT    GPIOA
#define LORA_AUX_PIN          GPIO_PIN_4
#define L610_RST_GPIO_PORT    GPIOB
#define L610_RST_PIN          GPIO_PIN_12



// 函数声明
void process_mqtt_message(const char *topic, const char *payload, UART_HandleTypeDef *lora_huart);
void dtu_process(UART_HandleTypeDef *dtu_huart, UART_HandleTypeDef *lora_huart);
void lora_process_rx(UART_HandleTypeDef *lora_huart, UART_HandleTypeDef *dtu_huart);
char *data_to_json(const DeviceDataPacket *data);
void upload_to_cloud(UART_HandleTypeDef *lte_huart, const char *json_data);



#endif

