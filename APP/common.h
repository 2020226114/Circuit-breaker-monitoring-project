#ifndef __COMMON_H__
#define __COMMON_H__



#include <stdint.h>

#define RS485_HUART &huart1
#define LORA_HUART &huart2
#define LTE_HUART &huart3
#define DTU_HUART &huart3


// 设备场景类型字符串定义
#define SCENE_ELECTRODE_STR    "电极制备"
#define SCENE_BATTERY_STR      "模组安装"
#define SCENE_CHARGE_STR       "充放电测试"

#define BREAKER_OPEN  0  // 分闸
#define BREAKER_CLOSE 1  // 合闸

// 通用数据结构
#pragma pack(push, 1)  // 1字节对齐
typedef struct {
    char     device_id[32];      // 设备标识
    uint16_t alarm_count;        // 报警次数
    uint16_t trip_count;         // 拉闸次数
    float    contact_temp[2];    // 双端触点温度
    uint8_t  breaker_state;      // 断路器状态(0:分闸 1:合闸)
    char     scene[20];          // 场景类型
} DeviceCommonData;

// 场景1: 电极制备
typedef struct {
    float motor_current;         // 电机运行电流
    float short_circuit_current; // 短路电流峰值
    float line_voltage;          // 线路电压
} SceneElectrodeData;

// 场景2: 电池模组
typedef struct {
    float ground_fault_current;  // 接地故障电流
    float phase_current[3];      // 三相电流[A,B,C]
    float surge_current;         // 浪涌电流峰值
} SceneBatteryData;

// 场景3: 充放电测试
typedef struct {
    float charge_discharge; // 充放电电流
    float battery_voltage;          // 电池电压
    float body_temp;                // 断路器本体温度
} SceneChargeData;

// 完整数据包
typedef struct {
    DeviceCommonData common;
    union {
        SceneElectrodeData electrode;
        SceneBatteryData   battery;
        SceneChargeData    charge;
    } scene_data;
} DeviceDataPacket;
#pragma pack(pop)

// Lora配置
#define LORA_NODE_ID_1 "NODE_ELECTRODE"
#define LORA_NODE_ID_2 "NODE_BATTERY"
#define LORA_NODE_ID_3 "NODE_CHARGE"


#endif

