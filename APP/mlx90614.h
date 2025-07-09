#ifndef _MLX90614_H_
#define _MLX90614_H_


#include "stm32g4xx_hal.h"

// SMBus 配置
#define ACK     0
#define NACK    1
#define SA      0x00  // 从机地址
#define RAM_ACCESS      0x00
#define EEPROM_ACCESS   0x20
#define RAM_TOBJ1       0x07  // 物体温度寄存器

// 红外传感器结构体
typedef struct {
    GPIO_TypeDef* scl_port;
    uint16_t scl_pin;
    GPIO_TypeDef* sda_port;
    uint16_t sda_pin;
} MLX90614_HandleTypeDef;


// 函数声明
void SMBus_Init(MLX90614_HandleTypeDef* sensor);
void SMBus_StartBit(MLX90614_HandleTypeDef* sensor);
void SMBus_StopBit(MLX90614_HandleTypeDef* sensor);
void SMBus_SendBit(MLX90614_HandleTypeDef* sensor, uint8_t bit_out);
uint8_t SMBus_SendByte(MLX90614_HandleTypeDef* sensor, uint8_t Tx_buffer);
uint8_t SMBus_ReceiveBit(MLX90614_HandleTypeDef* sensor);
uint8_t SMBus_ReceiveByte(MLX90614_HandleTypeDef* sensor, uint8_t ack_nack);
void SMBus_Delay(uint16_t time);
uint16_t SMBus_ReadMemory(MLX90614_HandleTypeDef* sensor, uint8_t slaveAddress, uint8_t command);
uint8_t PEC_Calculation(uint8_t* pec);
float SMBus_ReadTemp(MLX90614_HandleTypeDef* sensor);




#endif

