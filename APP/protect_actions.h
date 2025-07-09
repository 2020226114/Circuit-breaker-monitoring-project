#ifndef __PROTECT_ACTIONS_H__
#define __PROTECT_ACTIONS_H__



#include <common.h>
#include <stdbool.h>
#include "stm32g4xx_hal.h"


// ������ֵ����
// �缫�Ʊ�����
#define MOTOR_CURRENT_THRESHOLD      50.0f   // 50A
#define SHORT_CIRCUIT_THRESHOLD      100.0f   // ��·������ֵ
#define LINE_VOLTAGE_UPPER_THRESHOLD 250.0f   
#define LINE_VOLTAGE_LOWER_THRESHOLD 200.0f   

// ���ģ�鳡��
#define GROUND_FAULT_THRESHOLD       5.0f    // 30mA
#define PHASE_IMBALANCE_WARN         5.0f     // 5%
#define PHASE_IMBALANCE_TRIP         10.0f    // 10%
#define SURGE_DURATION_THRESHOLD     200      // 200ms
#define SURGE_CURRENT_THRESHOLD      20.0f    // 20A ����ʵ������޸�

// ��ŵ���Գ���
#define CHARGE_DISCHARGE_THRESHOLD   100.0f    // ʾ����ֵ
#define VOLTAGE_WAVE_THRESHOLD       5.0f     // 5%

// ͨ����ֵ
#define CONTACT_TEMP_THRESHOLD       80.0f    // �����¶���ֵ
#define BREAKER_BODY_TEMP_THRESHOLD  70.0f    // ��·�������¶���ֵ

// ������������
void check_and_apply_protection(DeviceDataPacket *packet,UART_HandleTypeDef *huart);

// ��·�����ƺ���
void trip_breaker(const uint8_t *address,UART_HandleTypeDef *huart);
void reset_breaker(const uint8_t *address,UART_HandleTypeDef *huart);


#endif


