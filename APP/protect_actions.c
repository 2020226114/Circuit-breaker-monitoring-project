#include <protect_actions.h>
#include "dlt645.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "stm32g4xx_hal.h"


extern UART_HandleTypeDef huart1, huart2, huart3;

// ��·����ַ (����ʵ���豸����)
static const uint8_t BREAKER_ADDRESS[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

// ��բ���ƺ���
void trip_breaker(const uint8_t *address,UART_HandleTypeDef *huart) {
    uint8_t frame[24];
    // �������֡��06 01 01 01 = ��բ����
    dlt645_build_control_frame(frame, address, 0x06, 0x01, 0x01, 0x01);
    send_dlt645_frame(frame,24); // ����24�ֽ�֡
//    printf("Sent trip command via DL/T645!\n");
}

// ��բ���ƺ���
void reset_breaker(const uint8_t *address,UART_HandleTypeDef *huart) {
    uint8_t frame[24];
    // �������֡��06 02 01 01 = ��բ����
    dlt645_build_control_frame(frame, address, 0x06, 0x02, 0x01, 0x01);
    send_dlt645_frame(frame,24); // ����24�ֽ�֡
//    printf("Sent reset command via DL/T645!\n");
}



// �������಻ƽ����
float calculate_imbalance(float phases[3]) {
    float avg = (phases[0] + phases[1] + phases[2]) / 3.0f;
    float max_dev = 0;

    for (int i = 0; i < 3; i++) {
        float dev = fabsf(phases[i] - avg);
        if (dev > max_dev) max_dev = dev;
    }

    return (max_dev / avg) * 100.0f;
}

// ��ӿ����
static uint32_t surge_start_time = 0;
static bool surge_detected = false;

// ��ӿ������⺯��
static void check_surge_current(DeviceDataPacket *packet,UART_HandleTypeDef *huart)
	{
		
float current = packet->scene_data.battery.surge_current;
		uint32_t current_time = HAL_GetTick();

if (current > SURGE_DURATION_THRESHOLD) {
    if (!surge_detected) {
        surge_start_time = current_time;
        surge_detected = true;
//        printf("Surge current detected: %.1fA\n", current);
         } else {
             uint32_t duration = current_time - surge_start_time;
             if (duration> SURGE_DURATION_THRESHOLD) {
                 trip_breaker((uint8_t *)BREAKER_ADDRESS,huart);
                 packet->common.trip_count++;
                 surge_detected = false;
//                 printf("Surge current trip! Duration: %dms\n",duration);
              }
          }
       } else if (surge_detected) {
            surge_detected = false;
    }
}

// ��ѹ���������Ҫ��ʷ���ݱȽ�
#define VOLTAGE_HISTORY_SIZE 10
static float voltage_history[VOLTAGE_HISTORY_SIZE] = {0};
static int voltage_index = 0;



// ��ѹ������������
static void analyze_voltage_wave(DeviceDataPacket *packet) {

    float current_voltage = packet->scene_data.charge.battery_voltage;

// ������ʷ����
    voltage_history[voltage_index] = current_voltage;
    voltage_index = (voltage_index + 1) % VOLTAGE_HISTORY_SIZE;

// ���㲨����
float min = voltage_history[0];
float max = voltage_history[0];
float sum = voltage_history[0];
int valid_count = 1;

        for (int i = 1; i < VOLTAGE_HISTORY_SIZE; i++) {
             if (voltage_history[i] > 0) {  // ֻ������Ч����
             if (voltage_history[i] < min) min = voltage_history[i];
             if (voltage_history[i] > max) max = voltage_history[i];
             sum += voltage_history[i];
             valid_count++;
                }
            }

     if (valid_count > 1) {
         float avg = sum / valid_count;
         float wave = ((max - min) / avg) * 100.0f;

         if (wave > VOLTAGE_WAVE_THRESHOLD) {
             packet->common.alarm_count++;
//             printf("Voltage wave detected: %.1f%% (%.1fV-%.1fV)\n",wave, min, max);
        }
    }
}
// ��鲢Ӧ�ñ�������
void check_and_apply_protection(DeviceDataPacket *packet,UART_HandleTypeDef *huart) {

    // ͨ�ñ����������¶�
    for (int i = 0; i < 2; i++) {
        if (packet->common.contact_temp[i] > CONTACT_TEMP_THRESHOLD) {
            trip_breaker((uint8_t *)BREAKER_ADDRESS,huart);
            packet->common.alarm_count++;
//            printf("Contact temp high: %.1fC\n", packet->common.contact_temp[i]);

            // ��������״̬ (���÷��Ѽ���)
            packet->common.breaker_state = BREAKER_OPEN;
            return;
        }
    }

    // �����ض�����
    if (strcmp(packet->common.scene, SCENE_ELECTRODE_STR) == 0) {
        // �缫�Ʊ�����
        if (packet->scene_data.electrode.motor_current > MOTOR_CURRENT_THRESHOLD) {
            trip_breaker((uint8_t *)BREAKER_ADDRESS,huart);
            packet->common.trip_count++;
//            printf("Motor current over: %.1fA\n", packet->scene_data.electrode.motor_current);
        }

        if (packet->scene_data.electrode.short_circuit_current > SHORT_CIRCUIT_THRESHOLD) {
            trip_breaker((uint8_t *)BREAKER_ADDRESS,huart);
            packet->common.trip_count++;
 //           printf("Short circuit: %.1fA\n", packet->scene_data.electrode.short_circuit_current);
        }

        if (packet->scene_data.electrode.line_voltage > LINE_VOLTAGE_UPPER_THRESHOLD ||
            packet->scene_data.electrode.line_voltage < LINE_VOLTAGE_LOWER_THRESHOLD) {
            packet->common.alarm_count++;
 //           printf("Voltage out of range: %.1fV\n", packet->scene_data.electrode.line_voltage);
        }
    }
    else if (strcmp(packet->common.scene, SCENE_BATTERY_STR) == 0) {
        // ���ģ�鳡��
        if (packet->scene_data.battery.ground_fault_current > GROUND_FAULT_THRESHOLD) {
            trip_breaker((uint8_t *)BREAKER_ADDRESS,huart);
 //           printf("Ground fault: %.3fA\n", packet->scene_data.battery.ground_fault_current);
        }

        float imbalance = calculate_imbalance(packet->scene_data.battery.phase_current);
        if (imbalance > PHASE_IMBALANCE_TRIP) {
            trip_breaker((uint8_t *)BREAKER_ADDRESS,huart);
            packet->common.trip_count++;
//            printf("Phase imbalance: %.1f%%\n", imbalance);
        }
        else if (imbalance > PHASE_IMBALANCE_WARN) {
            packet->common.alarm_count++;
 //           printf("Phase imbalance warning: %.1f%%\n", imbalance);
        }
        check_surge_current(packet,huart);
    }
    else if (strcmp(packet->common.scene, SCENE_CHARGE_STR) == 0) {
        // ��ŵ���Գ���
        if (packet->scene_data.charge.charge_discharge > CHARGE_DISCHARGE_THRESHOLD) {
            trip_breaker((uint8_t *)BREAKER_ADDRESS,huart);
            packet->common.trip_count++;
//            printf("Charge/discharge current over: %.1fA\n",packet->scene_data.charge.charge_discharge);
        }

        if (packet->scene_data.charge.body_temp > BREAKER_BODY_TEMP_THRESHOLD) {
            packet->common.alarm_count++;
 //           printf("Breaker body temp high: %.1fC\n", packet->scene_data.charge.body_temp);
        }
        analyze_voltage_wave(packet);
    }

}
