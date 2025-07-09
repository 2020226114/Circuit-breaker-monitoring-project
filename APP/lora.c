#include "lora.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



extern UART_HandleTypeDef huart1, huart2, huart3;


// ���⴫����ʵ��
MLX90614_HandleTypeDef ir_sensor1 = {
    .scl_port = GPIOB, .scl_pin = GPIO_PIN_4,
    .sda_port = GPIOB, .sda_pin = GPIO_PIN_3
};

MLX90614_HandleTypeDef ir_sensor2 = {
    .scl_port = GPIOC, .scl_pin = GPIO_PIN_11,
    .sda_port = GPIOC, .sda_pin = GPIO_PIN_12
};


DeviceDataPacket packet;  // ȫ���豸���ݰ�



// �豸���� (��¼ǰ���ݽڵ�����)
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

// ��·����ַ (����ʵ���豸����)
static const uint8_t BREAKER_ADDRESS[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

// �������
#define CMD_BUFFER_SIZE 64
static char cmd_buffer[CMD_BUFFER_SIZE];
static uint16_t cmd_index = 0;

// LoRa��ʼ��
void lora_init(UART_HandleTypeDef *lora_huart, UART_HandleTypeDef *rs485_huart) {
    // ��ʼ���豸ID�ͳ���
    strncpy(packet.common.device_id, DEVICE_ID, sizeof(packet.common.device_id));
    strncpy(packet.common.scene, SCENE_STR, sizeof(packet.common.scene));
	
		// ��ʼ��ͳ����Ϣ
    packet.common.alarm_count = 0;
    packet.common.trip_count = 0;
    packet.common.breaker_state = BREAKER_CLOSE;
    
    // ��ʼ���¶�����
    packet.common.contact_temp[0] = 0.0f;
    packet.common.contact_temp[1] = 0.0f;
	
	// ��ʼ�������ض�����
    memset(&packet.scene_data, 0, sizeof(packet.scene_data));
    
    HAL_GPIO_WritePin(RS485_RE_GPIO_PORT, RS485_RE_PIN, GPIO_PIN_SET); // Ĭ�Ͻ���ģʽ
    
    // ��ʼ�����⴫����
    SMBus_Init(&ir_sensor1);
    SMBus_Init(&ir_sensor2);
    
//    printf("LoRa module initialized. Device ID: %s, Scene: %s\r\n", DEVICE_ID, SCENE_STR);
}
	
	
	

// ��ȡ�����¶� (GY906)
float read_infrared_temp(MLX90614_HandleTypeDef *sensor) {
    return SMBus_ReadTemp(sensor);
}

// DL/T 645Э���ȡ��·������
int read_breaker_data(DeviceDataPacket *packet,UART_HandleTypeDef *rs485_huart)
{
  
    uint8_t tx_frame[24];
    uint8_t rx_buf[64];
    float value = 0.0f;
    //int ret = -1;
    uint16_t len = 0;
	
		// ���ó�����Ϣ
    strncpy(packet->common.scene, SCENE_STR, sizeof(packet->common.scene));

// ���ݳ�����ȡ��ͬ����
#ifdef SCENE_ELECTRODE
// ��ȡA����� (��ʶ: 02 02 01 00)
    dlt645_build_read_frame(tx_frame, BREAKER_ADDRESS, 0x02, 0x02, 0x01, 0x00);
    send_dlt645_frame(tx_frame, 16);
    HAL_Delay(50);

    if (HAL_UART_Receive(RS485_HUART, rx_buf, sizeof(rx_buf), 100) == HAL_OK) {
        len = sizeof(rx_buf);
        if (dlt645_parse_response(rx_buf, len, &value) == 0) {
            packet->scene_data.electrode.motor_current = value;
        }
    }

// ��ȡ�ߵ�ѹ (��ʶ: 02 01 01 FF)
    dlt645_build_read_frame(tx_frame, BREAKER_ADDRESS, 0x02, 0x01, 0x01, 0xFF);
    send_dlt645_frame(tx_frame, 16);
    HAL_Delay(50);

   
    if (HAL_UART_Receive(RS485_HUART, rx_buf, sizeof(rx_buf), 100) == HAL_OK) {
        len = sizeof(rx_buf);
        if (dlt645_parse_response(rx_buf, len, &value) == 0) {
            packet->scene_data.electrode.line_voltage = value;
        }
    }

// ��ȡ��·������ֵ (�Զ����ʶ: 04 00 01 05)
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
// ��ȡ������� (��ʶ: 02 02 01 FF)
    dlt645_build_read_frame(tx_frame, BREAKER_ADDRESS, 0x02, 0x02, 0x01, 0xFF);
    send_dlt645_frame(tx_frame, 16);
    HAL_Delay(50);

    if (HAL_UART_Receive(RS485_HUART, rx_buf, sizeof(rx_buf), 100) == HAL_OK) {
				len = sizeof(rx_buf);
				// �������(ʵ��Ӧ����Э�������������)
        uint8_t phase_data[6];
        memcpy(phase_data, &decoded[0], 6);
        packet->scene_data.battery.phase_current[0] = (phase_data[1]<<8 | phase_data[0])/10.0;
        packet->scene_data.battery.phase_current[1] = (phase_data[3]<<8 | phase_data[2])/10.0;
        packet->scene_data.battery.phase_current[2] = (phase_data[5]<<8 | phase_data[4])/10.0;
        }

// ��ȡ�ӵع��ϵ��� (�Զ����ʶ)
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
     // ��ȡ��ص�ѹ (�Զ����ʶ)
    dlt645_build_read_frame(tx_frame, BREAKER_ADDRESS, 0x04, 0x00, 0x01, 0x02);
    send_dlt645_frame(tx_frame, 16);
    HAL_Delay(50);

    if (HAL_UART_Receive(RS485_HUART, rx_buf, sizeof(rx_buf), 100) == HAL_OK) {
        len = sizeof(rx_buf);
        if (dlt645_parse_response(rx_buf, len, &value) == 0) {
            packet->scene_data.charge.battery_voltage = value;
        }
    }

// ��ȡ��ŵ���� (�Զ����ʶ)
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

		
    // ��ȡ��·��״̬
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

// ������յ�������
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
    
    // ����ACK��Ӧ
    char ack[32];
//    snprintf(ack, sizeof(ack), "ACK:%s:DONE\r\n", DEVICE_ID);
    HAL_GPIO_WritePin(LORA_AUX_GPIO_PORT, LORA_AUX_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_UART_Transmit(LORA_HUART, (uint8_t *)ack, strlen(ack), 100);
    HAL_Delay(10);
    HAL_GPIO_WritePin(LORA_AUX_GPIO_PORT, LORA_AUX_PIN, GPIO_PIN_SET);
}

// ����LoRa��������
static void process_lora_rx(UART_HandleTypeDef *lora_huart, UART_HandleTypeDef *rs485_huart) {
    uint8_t rx_char;
    
    // ����Ƿ������ݵ���
    while (HAL_UART_Receive(LORA_HUART, &rx_char, 1, 0) == HAL_OK) {
        // ��������������򻺳�����
        if (rx_char == '\n' || rx_char == '\r' || cmd_index >= CMD_BUFFER_SIZE - 1) {
            if (cmd_index > 0) {
                cmd_buffer[cmd_index] = '\0';
                
                // ��������ʽ
                if (strncmp(cmd_buffer, "CMD:", 4) == 0) {
                    char *device_id = cmd_buffer + 4;
                    char *cmd = strchr(device_id, ':');
                    
                    if (cmd) {
                        *cmd = '\0';
                        cmd++;
                        
                        // ����Ƿ��Ǳ��豸
                        if (strcmp(device_id, DEVICE_ID) == 0) {
                            process_command(cmd, lora_huart,RS485_HUART);
                        }
                    }
                }
            }
            cmd_index = 0;
        } 
        // �洢��Ч�ַ�
        else if (rx_char >= 32 && rx_char <= 126) {
            if (cmd_index < CMD_BUFFER_SIZE - 1) {
                cmd_buffer[cmd_index++] = rx_char;
            }
        }
    }
}


// LoRa���ݴ���������
void lora_process(UART_HandleTypeDef *lora_huart, UART_HandleTypeDef *rs485_huart) {
    static uint32_t last_send_time = 0;
    const uint32_t send_interval = 5000; // 5�뷢�ͼ��
    
    // 1. ������յ�������
    process_lora_rx(LORA_HUART, RS485_HUART);
    
    // 2. ����Ƿ񵽴﷢��ʱ��
    uint32_t current_time = HAL_GetTick();
    if (current_time - last_send_time >= send_interval) {
        last_send_time = current_time;
        
        // 3. ��ȡ��·������
        if (read_breaker_data(&packet, RS485_HUART) != 0) {
//            printf("Error reading breaker data\r\n");
        }
        
        // 4. ��ȡ�¶�����
        packet.common.contact_temp[0] = read_infrared_temp(&ir_sensor1);
        packet.common.contact_temp[1] = read_infrared_temp(&ir_sensor2);
        
        // 5. ִ�б������
        check_and_apply_protection(&packet, RS485_HUART);
        
        // 6. ����ͳ����Ϣ
        packet.common.alarm_count++;
        packet.common.trip_count += 2;
        
        // 7. ��������
        lora_send_data(LORA_HUART, &packet);
    }
}


// ����LoRa����
void lora_send_data(UART_HandleTypeDef *lora_huart, DeviceDataPacket *packet) {
    // ���AUX����״̬��ģ���Ƿ���У�
    if (HAL_GPIO_ReadPin(LORA_AUX_GPIO_PORT, LORA_AUX_PIN) == GPIO_PIN_SET) {
        // ȷ��AUX����Ϊ�ͣ�����ģʽ��
        HAL_GPIO_WritePin(LORA_AUX_GPIO_PORT, LORA_AUX_PIN, GPIO_PIN_RESET);
        HAL_Delay(1);
        
        // ��������
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
        
        // �ָ�����ģʽ
        HAL_Delay(1);
        HAL_GPIO_WritePin(LORA_AUX_GPIO_PORT, LORA_AUX_PIN, GPIO_PIN_SET);
        
}
		
}

