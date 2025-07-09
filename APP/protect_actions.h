#ifndef __PROTECT_ACTIONS_H__
#define __PROTECT_ACTIONS_H__



#include <common.h>
#include <stdbool.h>
#include "stm32g4xx_hal.h"


// 保护阈值定义
// 电极制备场景
#define MOTOR_CURRENT_THRESHOLD      50.0f   // 50A
#define SHORT_CIRCUIT_THRESHOLD      100.0f   // 短路电流阈值
#define LINE_VOLTAGE_UPPER_THRESHOLD 250.0f   
#define LINE_VOLTAGE_LOWER_THRESHOLD 200.0f   

// 电池模组场景
#define GROUND_FAULT_THRESHOLD       5.0f    // 30mA
#define PHASE_IMBALANCE_WARN         5.0f     // 5%
#define PHASE_IMBALANCE_TRIP         10.0f    // 10%
#define SURGE_DURATION_THRESHOLD     200      // 200ms
#define SURGE_CURRENT_THRESHOLD      20.0f    // 20A 根据实际情况修改

// 充放电测试场景
#define CHARGE_DISCHARGE_THRESHOLD   100.0f    // 示例阈值
#define VOLTAGE_WAVE_THRESHOLD       5.0f     // 5%

// 通用阈值
#define CONTACT_TEMP_THRESHOLD       80.0f    // 触点温度阈值
#define BREAKER_BODY_TEMP_THRESHOLD  70.0f    // 断路器本体温度阈值

// 保护动作函数
void check_and_apply_protection(DeviceDataPacket *packet,UART_HandleTypeDef *huart);

// 断路器控制函数
void trip_breaker(const uint8_t *address,UART_HandleTypeDef *huart);
void reset_breaker(const uint8_t *address,UART_HandleTypeDef *huart);


#endif


