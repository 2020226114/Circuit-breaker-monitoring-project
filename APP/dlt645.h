#ifndef __DLT645_H__
#define __DLT645_H__

#include "stm32g4xx_hal.h"
#include <string.h>

// 定义RS485控制引脚
#define RS485_RE_PIN_GPIO_PORT GPIOB
#define RS485_RE_PIN_GPIO_PIN  GPIO_PIN_5

// 构造读取数据帧 (20字节缓冲区)
void dlt645_build_read_frame(uint8_t *frame, const uint8_t *address,
                            uint8_t di3, uint8_t di2, uint8_t di1, uint8_t di0);

//控制帧构造和发送函数声明
void dlt645_build_control_frame(uint8_t *frame, const uint8_t *address,
                               uint8_t di3, uint8_t di2, uint8_t di1, uint8_t di0);

void send_dlt645_frame(uint8_t *frame, size_t len);

// 解析响应帧
int dlt645_parse_response(const uint8_t *data, size_t len, float *value);


#endif
