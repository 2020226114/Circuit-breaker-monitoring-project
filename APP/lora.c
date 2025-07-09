#include "lora.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



extern UART_HandleTypeDef huart1, huart2, huart3;


// 红外传感器实例
MLX90614_HandleTypeDef ir_sensor1 = {
    .scl_port = GPIOB, .scl_pin = GPIO_PIN_4,
    .sda_port = GPIOB, .sda_pin = GPIO_PIN_3
};

MLX90614_HandleTypeDef ir_sensor2 = {
    .scl_port = GPIOC, .scl_pin = GPIO_PIN_11,
    .sda_port = GPIOC, .sda_pin = GPIO_PIN_12
};


DeviceDataPacket packet;  // 全局设备数据包



// 设备配置 (烧录前根据节点设置)
#ifdef SCENE_ELECTRODE
    static const char DEVICE_ID[] = LORA_NODE_ID_1;
    static const char SCENE_STR[] = SCENE_ELECTRODE_STR;
#elif defined(SCENE_BATTERY)
    static const char DEVICE_ID[] = LORA_NODE_ID_2;
    static const char SCENE_STR[] = SCENE_BATTERY_STR;
#else
    static const char DEVICE_ID[] = LORA_NODE_ID_3;
    static const char SCENE_STR[] = SCENE_CHARGE_STR;
#endif

// 断路器地址 (根据实际设备配置)
static const uint8_t BREAKER_ADDRESS[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

// 命令缓冲区
#define CMD_BUFFER_SIZE 64
static char cmd_buffer[CMD_BUFFER_SIZE];
static uint16_t cmd_index = 0;

// LoRa初始化
void lora_init(UART_HandleTypeDef *lora_huart, UART_HandleTypeDef *rs485_huart) {
    // 初始化设备ID和场景
    strncpy(packet.common.device_id, DEVICE_ID, sizeof(packet.common.device_id));
    strncpy(packet.common.scene, SCENE_STR, sizeof(packet.common.scene));
	
		// 初始化统计信息
    packet.common.alarm_count = 0;
    packet.common.trip_count = 0;
    packet.common.breaker_state = BREAKER_CLOSE;
    
    // 初始化温度数据
    packet.common.contact_temp[0] = 0.0f;
    packet.common.contact_temp[1] = 0.0f;
	
	// 初始化场景特定数据
    memset(&packet.scene_data, 0, sizeof(packet.scene_data));
    
    HAL_GPIO_WritePin(RS485_RE_GPIO_PORT, RS485_RE_PIN, GPIO_PIN_SET); // 默认接收模式
    
    // 初始化红外传感器
    SMBus_Init(&ir_sensor1);
    SMBus_Init(&ir_sensor2);
    
//    printf("LoRa module initialized. Device ID: %s, Scene: %s\r\n", DEVICE_ID, SCENE_STR);
}
	
	
	

// 读取红外温度 (GY906)
float read_infrared_temp(MLX90614_HandleTypeDef *sensor) {
    return SMBus_ReadTemp(sensor);
}

// DL/T 645协议读取断路器参数
int read_breaker_data(DeviceDataPacket *packet,UART_HandleTypeDef *rs485_huart)
{
  
    uint8_t tx_frame[24];
    uint8_t rx_buf[64];
    float value = 0.0f;
    //int ret = -1;
    uint16_t len = 0;
	
		// 设置场景信息
    strncpy(packet->common.scene, SCENE_STR, sizeof(packet->common.scene));

// 根据场景读取不同数据
#ifdef SCENE_ELECTRODE
// 读取A相电流 (标识: 02 02 01 00)
    dlt645_build_read_frame(tx_frame, BREAKER_ADDRESS, 0x02, 0x02, 0x01, 0x00);
    send_dlt645_frame(tx_frame, 16);
    HAL_Delay(50);

    if (HAL_UART_Receive(RS485_HUART, rx_buf, sizeof(rx_buf), 100) == HAL_OK) {
        len = sizeof(rx_buf);
        if (dlt645_parse_response(rx_buf, len, &value) == 0) {
            packet->scene_data.electrode.motor_current = value;
        }
    }

// 读取线电压 (标识: 02 01 01 FF)
    dlt645_build_read_frame(tx_frame, BREAKER_ADDRESS, 0x02, 0x01, 0x01, 0xFF);
    send_dlt645_frame(tx_frame, 16);
    HAL_Delay(50);

   
    if (HAL_UART_Receive(RS485_HUART, rx_buf, sizeof(rx_buf), 100) == HAL_OK) {
        len = sizeof(rx_buf);
        if (dlt645_parse_response(rx_buf, len, &value) == 0) {
            packet->scene_data.electrode.line_voltage = value;
        }
    }

// 读取短路电流峰值 (自定义标识: 04 00 01 05)
    dlt645_build_read_frame(tx_frame, BREAKER_ADDRESS, 0x04, 0x00, 0x01, 0x05);
    send_dlt645_frame( tx_frame, 16);
    HAL_Delay(50);

    if (HAL_UART_Receive(RS485_HUART, rx_buf, sizeof(rx_buf), 100) == HAL_OK) {
        len = sizeof(rx_buf);
        if (dlt645_parse_response(rx_buf, len, &value) == 0) {
            packet->scene_data.electrode.short_circuit_current = value;
//            printf("Short circuit current: %.1fA\r\n", value);
        }
    }
#elif defined(SCENE_BATTERY)
// 读取三相电流 (标识: 02 02 01 FF)
    dlt645_build_read_frame(tx_frame, BREAKER_ADDRESS, 0x02, 0x02, 0x01, 0xFF);
    send_dlt645_frame(tx_frame, 16);
    HAL_Delay(50);

    if (HAL_UART_Receive(RS485_HUART, rx_buf, sizeof(rx_buf), 100) == HAL_OK) {
				len = sizeof(rx_buf);
				// 三相电流(实际应根据协议解析三相数据)
        uint8_t phase_data[6];
        memcpy(phase_data, &decoded[0], 6);
        packet->scene_data.battery.phase_current[0] = (phase_data[1]<<8 | phase_data[0])/10.0;
        packet->scene_data.battery.phase_current[1] = (phase_data[3]<<8 | phase_data[2])/10.0;
        packet->scene_data.battery.phase_current[2] = (phase_data[5]<<8 | phase_data[4])/10.0;
        }

// 读取接地故障电流 (自定义标识)
    dlt645_build_read_frame(tx_frame, BREAKER_ADDRESS, 0x04, 0x00, 0x01, 0x01);
    send_dlt645_frame(tx_frame, 16);
    HAL_Delay(50);

     if (HAL_UART_Receive(RS485_HUART, rx_buf, sizeof(rx_buf), 100) == HAL_OK) {
        len = sizeof(rx_buf);
        if (dlt645_parse_response(rx_buf, len, &value) == 0) {
            packet->scene_data.battery.ground_fault_current = value;
        }
    }

#else // SCENE_CHARGE
     // 读取电池电压 (自定义标识)
    dlt645_build_read_frame(tx_frame, BREAKER_ADDRESS, 0x04, 0x00, 0x01, 0x02);
    send_dlt645_frame(tx_frame, 16);
    HAL_Delay(50);

    if (HAL_UART_Receive(RS485_HUART, rx_buf, sizeof(rx_buf), 100) == HAL_OK) {
        len = sizeof(rx_buf);
        if (dlt645_parse_response(rx_buf, len, &value) == 0) {
            packet->scene_data.charge.battery_voltage = value;
        }
    }

// 读取充放电电流 (自定义标识)
    dlt645_build_read_frame(tx_frame, BREAKER_ADDRESS, 0x04, 0x00, 0x01, 0x03);
    send_dlt645_frame(tx_frame, 16);
    HAL_Delay(50);

    if (HAL_UART_Receive(RS485_HUART, rx_buf, sizeof(rx_buf), 100) == HAL_OK) {
        len = sizeof(rx_buf);
        if (dlt645_parse_response(rx_buf, len, &value) == 0) {
            packet->scene_data.charge.charge_discharge = value;
        }
    }
		
#endif

		
    // 读取断路器状态
    dlt645_build_read_frame(tx_frame, BREAKER_ADDRESS, 0x00, 0x01, 0x01, 0x00);
    send_dlt645_frame(tx_frame, 16);
    HAL_Delay(50);

    if (HAL_UART_Receive(RS485_HUART, rx_buf, sizeof(rx_buf), 100) == HAL_OK) {
        len = sizeof(rx_buf);
        if (len > 0 && rx_buf[10] == 0x01) {
            packet->common.breaker_state = (rx_buf[11] == 0x01) ? BREAKER_CLOSE : BREAKER_OPEN;
        }
    }

    return 0;
}

// 处理接收到的命令
static void process_command(const char *cmd, UART_HandleTypeDef *lora_huart, 
                           UART_HandleTypeDef *rs485_huart) {
//    printf("Received command: %s\r\n", cmd);
    
    if (strcmp(cmd, "REBOOT") == 0) {
//        printf("Rebooting system...\r\n");
        HAL_Delay(100);
        NVIC_SystemReset();
    }
    else if (strcmp(cmd, "RESET") == 0) {
        reset_breaker((uint8_t *)BREAKER_ADDRESS, RS485_HUART);
        packet.common.breaker_state = BREAKER_CLOSE;
//        printf("Breaker reset command executed\r\n");
    }
    else if (strcmp(cmd, "TRIP") == 0) {
        trip_breaker((uint8_t *)BREAKER_ADDRESS, RS485_HUART);
        packet.common.breaker_state = BREAKER_OPEN;
//        printf("Breaker trip command executed\r\n");
    }
    else {
//        printf("Unknown command: %s\r\n", cmd);
    }
    
    // 发送ACK响应
    char ack[32];
//    snprintf(ack, sizeof(ack), "ACK:%s:DONE\r\n", DEVICE_ID);
    HAL_GPIO_WritePin(LORA_AUX_GPIO_PORT, LORA_AUX_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_UART_Transmit(LORA_HUART, (uint8_t *)ack, strlen(ack), 100);
    HAL_Delay(10);
    HAL_GPIO_WritePin(LORA_AUX_GPIO_PORT, LORA_AUX_PIN, GPIO_PIN_SET);
}

// 处理LoRa接收数据
static void process_lora_rx(UART_HandleTypeDef *lora_huart, UART_HandleTypeDef *rs485_huart) {
    uint8_t rx_char;
    
    // 检查是否有数据到达
    while (HAL_UART_Receive(LORA_HUART, &rx_char, 1, 0) == HAL_OK) {
        // 处理命令结束符或缓冲区满
        if (rx_char == '\n' || rx_char == '\r' || cmd_index >= CMD_BUFFER_SIZE - 1) {
            if (cmd_index > 0) {
                cmd_buffer[cmd_index] = '\0';
                
                // 检查命令格式
                if (strncmp(cmd_buffer, "CMD:", 4) == 0) {
                    char *device_id = cmd_buffer + 4;
                    char *cmd = strchr(device_id, ':');
                    
                    if (cmd) {
                        *cmd = '\0';
                        cmd++;
                        
                        // 检查是否是本设备
                        if (strcmp(device_id, DEVICE_ID) == 0) {
                            process_command(cmd, lora_huart,RS485_HUART);
                        }
                    }
                }
            }
            cmd_index = 0;
        } 
        // 存储有效字符
        else if (rx_char >= 32 && rx_char <= 126) {
            if (cmd_index < CMD_BUFFER_SIZE - 1) {
                cmd_buffer[cmd_index++] = rx_char;
            }
        }
    }
}


// LoRa数据处理主函数
void lora_process(UART_HandleTypeDef *lora_huart, UART_HandleTypeDef *rs485_huart) {
    static uint32_t last_send_time = 0;
    const uint32_t send_interval = 5000; // 5秒发送间隔
    
    // 1. 处理接收到的命令
    process_lora_rx(LORA_HUART, RS485_HUART);
    
    // 2. 检查是否到达发送时间
    uint32_t current_time = HAL_GetTick();
    if (current_time - last_send_time >= send_interval) {
        last_send_time = current_time;
        
        // 3. 读取断路器数据
        if (read_breaker_data(&packet, RS485_HUART) != 0) {
//            printf("Error reading breaker data\r\n");
        }
        
        // 4. 读取温度数据
        packet.common.contact_temp[0] = read_infrared_temp(&ir_sensor1);
        packet.common.contact_temp[1] = read_infrared_temp(&ir_sensor2);
        
        // 5. 执行保护检查
        check_and_apply_protection(&packet, RS485_HUART);
        
        // 6. 更新统计信息
        packet.common.alarm_count++;
        packet.common.trip_count += 2;
        
        // 7. 发送数据
        lora_send_data(LORA_HUART, &packet);
    }
}


// 发送LoRa数据
void lora_send_data(UART_HandleTypeDef *lora_huart, DeviceDataPacket *packet) {
    // 检查AUX引脚状态（模块是否空闲）
    if (HAL_GPIO_ReadPin(LORA_AUX_GPIO_PORT, LORA_AUX_PIN) == GPIO_PIN_SET) {
        // 确保AUX引脚为低（发送模式）
        HAL_GPIO_WritePin(LORA_AUX_GPIO_PORT, LORA_AUX_PIN, GPIO_PIN_RESET);
        HAL_Delay(1);
        
        // 发送数据
//        HAL_StatusTypeDef status = HAL_UART_Transmit(
//            LORA_HUART, 
//            (uint8_t *)packet, 
//            sizeof(DeviceDataPacket), 
//            100
//        );
			    HAL_StatusTypeDef status = HAL_UART_Transmit(
          LORA_HUART, 
          (uint8_t*)"HELLO WORLD", 
          sizeof"HELLO WORLD", 
          100
        );
        
        // 恢复接收模式
        HAL_Delay(1);
        HAL_GPIO_WritePin(LORA_AUX_GPIO_PORT, LORA_AUX_PIN, GPIO_PIN_SET);
        
}
		
}

