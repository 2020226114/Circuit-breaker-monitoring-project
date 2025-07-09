#ifndef __COMMON_H__
#define __COMMON_H__



#include <stdint.h>

#define RS485_HUART &huart1
#define LORA_HUART &huart2
#define LTE_HUART &huart3
#define DTU_HUART &huart3


// �豸���������ַ�������
#define SCENE_ELECTRODE_STR    "�缫�Ʊ�"
#define SCENE_BATTERY_STR      "ģ�鰲װ"
#define SCENE_CHARGE_STR       "��ŵ����"

#define BREAKER_OPEN  0  // ��բ
#define BREAKER_CLOSE 1  // ��բ

// ͨ�����ݽṹ
#pragma pack(push, 1)  // 1�ֽڶ���
typedef struct {
    char     device_id[32];      // �豸��ʶ
    uint16_t alarm_count;        // ��������
    uint16_t trip_count;         // ��բ����
    float    contact_temp[2];    // ˫�˴����¶�
    uint8_t  breaker_state;      // ��·��״̬(0:��բ 1:��բ)
    char     scene[20];          // ��������
} DeviceCommonData;

// ����1: �缫�Ʊ�
typedef struct {
    float motor_current;         // ������е���
    float short_circuit_current; // ��·������ֵ
    float line_voltage;          // ��·��ѹ
} SceneElectrodeData;

// ����2: ���ģ��
typedef struct {
    float ground_fault_current;  // �ӵع��ϵ���
    float phase_current[3];      // �������[A,B,C]
    float surge_current;         // ��ӿ������ֵ
} SceneBatteryData;

// ����3: ��ŵ����
typedef struct {
    float charge_discharge; // ��ŵ����
    float battery_voltage;          // ��ص�ѹ
    float body_temp;                // ��·�������¶�
} SceneChargeData;

// �������ݰ�
typedef struct {
    DeviceCommonData common;
    union {
        SceneElectrodeData electrode;
        SceneBatteryData   battery;
        SceneChargeData    charge;
    } scene_data;
} DeviceDataPacket;
#pragma pack(pop)

// Lora����
#define LORA_NODE_ID_1 "NODE_ELECTRODE"
#define LORA_NODE_ID_2 "NODE_BATTERY"
#define LORA_NODE_ID_3 "NODE_CHARGE"


#endif

