#include "gateway.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


extern UART_HandleTypeDef huart1, huart2, huart3;

// 设备状态跟踪
typedef struct {
    char device_id[32];
    uint32_t last_upload;
} DeviceState;



static DeviceState devices[3] = {0};

/* MQTT消息处理函数 */
void process_mqtt_message(const char *topic, const char *payload, UART_HandleTypeDef *lora_huart) {
//    printf("Received MQTT message: [%s] %s\r\n", topic, payload);

    // 检查是否为支持的三个主题之一
    const char *charge_topic = "$oc/devices/68579546d582f2001833de5a_charge_discharge/sys/messages/down";
    const char *electrode_topic = "$oc/devices/68579546d582f2001833de5a_electrode_preparation/sys/messages/down";
    const char *module_topic = "$oc/devices/68579546d582f2001833de5a_module_installation/sys/messages/down";

    if (strcmp(topic, charge_topic) != 0 &&
        strcmp(topic, electrode_topic) != 0 &&
        strcmp(topic, module_topic) != 0) {
        return; // 忽略不相关主题
    }

    // 解析控制命令
    cJSON *root = cJSON_Parse(payload);
    if (!root) {
//        printf("Invalid JSON format\r\n");
        return;
    }

    // 解析 device_id
    cJSON *device_id_obj = cJSON_GetObjectItem(root, "device_id");
    if (!device_id_obj || !cJSON_IsString(device_id_obj)) {
//        printf("Missing device_id\r\n");
        cJSON_Delete(root);
        return;
    }
//    const char *device_id = device_id_obj->valuestring;

    // 解析控制状态
    cJSON *state_obj = cJSON_GetObjectItem(root, "control_breaker_state");
    if (!state_obj || !cJSON_IsNumber(state_obj)) {
//        printf("Missing control_breaker_state\r\n");
        cJSON_Delete(root);
        return;
    }
//    int state = state_obj->valueint;

//    printf("Device command: %s -> %s\r\n",          device_id, state == 0 ? "TRIP" : "RESET");

    // 构造LoRa命令帧
    uint8_t lora_cmd[64];
//    int len = snprintf((char*)lora_cmd, sizeof(lora_cmd),               "CMD:%s:%s", device_id, state == 0 ? "TRIP" : "RESET");

    // 通过LoRa发送
    if (LORA_HUART) {
        // 设置发送模式
        HAL_GPIO_WritePin(LORA_AUX_GPIO_PORT, LORA_AUX_PIN, GPIO_PIN_RESET);
        HAL_Delay(10);

        // 发送命令
        HAL_StatusTypeDef status = HAL_UART_Transmit(
            LORA_HUART, 
            lora_cmd, 
            sizeof(lora_cmd), 
            100
        );
        
        if (status == HAL_OK) {
//            printf("LoRa command sent: %d bytes\r\n", len);
        } else {
   //         printf("Failed to send LoRa command. Error: %d\r\n", status);
        }

        // 恢复接收模式
        HAL_Delay(50);
        HAL_GPIO_WritePin(LORA_AUX_GPIO_PORT, LORA_AUX_PIN, GPIO_PIN_SET);
    }

    cJSON_Delete(root);
}

// 处理4G模块接收数据
void dtu_process(UART_HandleTypeDef *dtu_huart, UART_HandleTypeDef *lora_huart) {
    static char buffer[256] = {0};
    static uint16_t buf_index = 0;
    uint8_t rx_char;
    
    // 检查是否有数据到达
    if (HAL_UART_Receive(DTU_HUART, &rx_char, 1, 0) == HAL_OK) {
        // 处理完整消息或缓冲区满
        if (rx_char == '\n' || buf_index >= sizeof(buffer) - 1) {
            if (buf_index > 0) {
                buffer[buf_index] = '\0';
                
                // 华为云下行消息格式解析
                char *topic_start = strstr(buffer, "\"topic\":\"");
                char *payload_start = strstr(buffer, "\"payload\":");
                
                if (topic_start && payload_start) {
                    // 提取主题
                    char *topic = topic_start + 9; // 跳过"topic":"
                    char *topic_end = strchr(topic, '"');
                    if (topic_end) *topic_end = '\0';
                    
                    // 提取payload
                    char *payload = payload_start + 10; // 跳过"payload":
                    char *payload_end = strchr(payload, '}');
                    if (payload_end) *(payload_end+1) = '\0'; // 包含结束大括号
                    
                    // 处理MQTT消息
                    process_mqtt_message(topic, payload,LORA_HUART);
                }
            }
            buf_index = 0;
        } 
        // 存储有效字符
        else if (rx_char >= 32 && rx_char <= 126) {
            buffer[buf_index++] = rx_char;
        }
    }
}



// 转换数据为JSON格式
char *data_to_json(const DeviceDataPacket *data) {
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "services", services);

    cJSON *service_item = cJSON_CreateObject();
    cJSON_AddItemToArray(services, service_item);

    // 根据场景设置service_id
    const char *service_id = "";
    if (strcmp(data->common.scene, SCENE_ELECTRODE_STR) == 0) {
        service_id = "electrode_preparation";
    } else if (strcmp(data->common.scene, SCENE_BATTERY_STR) == 0) {
        service_id = "module_installation";
    } else {
        service_id = "charge_discharge";
    }
    cJSON_AddStringToObject(service_item, "service_id", service_id);

    cJSON *properties = cJSON_CreateObject();
    cJSON_AddItemToObject(service_item, "properties", properties);

    // 添加公共属性
    cJSON_AddStringToObject(properties, "device_id", data->common.device_id);
    cJSON_AddNumberToObject(properties, "alarm_count", data->common.alarm_count);
    cJSON_AddNumberToObject(properties, "trip_count", data->common.trip_count);

    // 温度数组
    cJSON *temp_arr = cJSON_CreateArray();
    cJSON_AddItemToArray(temp_arr, cJSON_CreateNumber(data->common.contact_temp[0]));
    cJSON_AddItemToArray(temp_arr, cJSON_CreateNumber(data->common.contact_temp[1]));
    cJSON_AddItemToObject(properties, "contact_temp", temp_arr);

    // 断路器状态 (0:分闸, 1:合闸)
    cJSON_AddNumberToObject(properties, "breaker_state", data->common.breaker_state);

    // 场景特定属性
    if (strcmp(data->common.scene, SCENE_ELECTRODE_STR) == 0) {
        cJSON_AddNumberToObject(properties, "motor_current", data->scene_data.electrode.motor_current);
        cJSON_AddNumberToObject(properties, "short_circuit", data->scene_data.electrode.short_circuit_current);
        cJSON_AddNumberToObject(properties, "line_voltage", data->scene_data.electrode.line_voltage);
    } else if (strcmp(data->common.scene, SCENE_BATTERY_STR) == 0) {
        cJSON_AddNumberToObject(properties, "ground_fault", data->scene_data.battery.ground_fault_current);
        cJSON_AddNumberToObject(properties, "surge_current", data->scene_data.battery.surge_current);

        cJSON *phase_arr = cJSON_CreateArray();
        for (int i = 0; i < 3; i++) {
            cJSON_AddItemToArray(phase_arr, cJSON_CreateNumber(data->scene_data.battery.phase_current[i]));
        }
        cJSON_AddItemToObject(properties, "phase_currents", phase_arr);
    } else {
        cJSON_AddNumberToObject(properties, "chg_dchg_current", data->scene_data.charge.charge_discharge);
        cJSON_AddNumberToObject(properties, "battery_voltage", data->scene_data.charge.battery_voltage);
        cJSON_AddNumberToObject(properties, "body_temp", data->scene_data.charge.body_temp);
    }

    // 添加event_time字段
    cJSON_AddNullToObject(service_item, "event_time");

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_str;
}




// 通过4G上传数据
void upload_to_cloud(UART_HandleTypeDef *lte_huart, const char *json_data) {

		
    HAL_UART_Transmit(LTE_HUART, (uint8_t *)json_data, strlen(json_data), 1000);
}

// 处理LoRa接收数据
void lora_process_rx(UART_HandleTypeDef *lora_huart, UART_HandleTypeDef *lte_huart) {
    static char buffer[128] = {0};
    static uint16_t buf_index = 0;
    uint8_t rx_char;
    
    // 检查是否有数据到达
    while (HAL_UART_Receive(LORA_HUART, &rx_char, 1, 0) == HAL_OK) {
        // 处理完整消息或缓冲区满
        if (buf_index >= sizeof(buffer) - 1) {
            buf_index = 0; // 重置缓冲区
        }
        
        buffer[buf_index++] = rx_char;
        
        // 处理命令响应
        if (buf_index > 4 && strncmp(buffer, "ACK:", 4) == 0) {
  //          printf("Received ACK: %s\r\n", buffer);
            buf_index = 0;
        } 
        // 处理数据包
        else if (buf_index == sizeof(DeviceDataPacket)) {
            DeviceDataPacket packet;
            memcpy(&packet, buffer, sizeof(DeviceDataPacket));
            
  //          printf("Received data from %s [%s]\r\n",               packet.common.device_id, packet.common.scene);
            
            // 查找或注册设备
            int index = -1;
            for (int i = 0; i < 3; i++) {
                if (strcmp(devices[i].device_id, packet.common.device_id) == 0) {
                    index = i;
                    break;
                }
                else if (devices[i].device_id[0] == '\0') {
                    strncpy(devices[i].device_id, packet.common.device_id,
                            sizeof(devices[i].device_id));
                    devices[i].last_upload = 0;
                    index = i;
                    break;
                }
            }
            
       
                    char *json_data = data_to_json(&packet);
                    upload_to_cloud(LTE_HUART, json_data);
                    free(json_data);
                  
                }
            }
            
            buf_index = 0;
        }
    

