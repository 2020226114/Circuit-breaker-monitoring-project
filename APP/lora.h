#ifndef _LORA_H_
#define _LORA_H_


#include "stm32g4xx_hal.h"
#include "common.h"
#include "dlt645.h"
#include "protect_actions.h"
#include "mlx90614.h"

// 引脚定义
#define LORA_AUX_GPIO_PORT    GPIOA
#define LORA_AUX_PIN          GPIO_PIN_4
#define RS485_RE_GPIO_PORT    GPIOB
#define RS485_RE_PIN          GPIO_PIN_5

// 红外传感器实例
extern MLX90614_HandleTypeDef ir_sensor1;
extern MLX90614_HandleTypeDef ir_sensor2;

// 设备数据包
extern DeviceDataPacket packet;


// 函数声明
void lora_init(UART_HandleTypeDef *lora_huart, UART_HandleTypeDef *rs485_huart);
void lora_process(UART_HandleTypeDef *lora_huart, UART_HandleTypeDef *rs485_huart);
int read_breaker_data(DeviceDataPacket *packet, UART_HandleTypeDef *rs485_huart);
float read_infrared_temp(MLX90614_HandleTypeDef *sensor);
void lora_send_data(UART_HandleTypeDef *lora_huart, DeviceDataPacket *packet);



#endif

