/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32g4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lora.h"
#include "gateway.h"
#include "protect_actions.h"
#include "stdint.h"
#include "dlt645.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifdef SCENE_ELECTRODE
    static const char DEVICE_ID[] = LORA_NODE_ID_1;
#elif defined(SCENE_BATTERY)
    static const char DEVICE_ID[] = LORA_NODE_ID_2;
#else
    static const char DEVICE_ID[] = LORA_NODE_ID_3;
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t tx_frame[24];
static uint32_t current_command = 0;
static const uint8_t BREAKER_ADDRESS[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// ���Ͷ�ȡ����ĺ�����Ҫ����
void send_read_command(uint8_t di3, uint8_t di2, uint8_t di1, uint8_t di0) {
    current_command = (di3 << 24) | (di2 << 16) | (di1 << 8) | di0;
    dlt645_build_read_frame(tx_frame, BREAKER_ADDRESS, di3, di2, di1, di0);
    send_dlt645_frame(tx_frame, 16);
}
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32G4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt / USART1 wake-up interrupt through EXTI line 25.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */
//  �����ж� 
  if (__HAL_UART_GET_FLAG(RS485_HUART, UART_FLAG_RXNE))
  {
    uint8_t data = (uint8_t)(huart1.Instance->RDR & 0xFF);
    
    // �����յ������ݴ��ݸ�DL/T645������
    static uint8_t dlt645_rx_buf[64];
    static uint8_t dlt645_index = 0;
    
    if (dlt645_index < sizeof(dlt645_rx_buf) - 1) {
      dlt645_rx_buf[dlt645_index++] = data;
      
      // ���֡������(0x16)
      if (data == 0x16) {
        // ����֡������ɣ����н���
        float value;
        int status = dlt645_parse_response(dlt645_rx_buf, dlt645_index, &value);
       if (status == 0) {
    // �����ɹ������¶�·������
    #ifdef SCENE_ELECTRODE
    // �缫�Ʊ�����
    switch (current_command) {
        case 0x02020100: // A�����
            packet.scene_data.electrode.motor_current = value;
            break;
        case 0x020101FF: // �ߵ�ѹ
            packet.scene_data.electrode.line_voltage = value;
            break;
        case 0x04000105: // ��·����
            packet.scene_data.electrode.short_circuit_current = value;
            break;
        case 0x00010100: // ��·��״̬
            packet.common.breaker_state = (value == 1) ? BREAKER_CLOSE : BREAKER_OPEN;
            break;
    }
    #elif defined(SCENE_BATTERY)
    // ���ģ�鰲װ����
    switch (current_command) {
        case 0x020201FF: // �������
            // ���践�ص�����������Ĵ������
            packet.scene_data.battery.phase_current[0] = (value >> 16) & 0xFFFF;
            packet.scene_data.battery.phase_current[1] = (value >> 8) & 0xFF;
            packet.scene_data.battery.phase_current[2] = value & 0xFF;
            break;
        case 0x04000101: // �ӵع��ϵ���
            packet.scene_data.battery.ground_fault_current = value;
            break;
        case 0x00010100: // ��·��״̬
            packet.common.breaker_state = (value == 1) ? BREAKER_CLOSE : BREAKER_OPEN;
            break;
    }
    #else
    // ��ŵ���Գ���
    switch (current_command) {
        case 0x04000102: // ��ص�ѹ
            packet.scene_data.charge.battery_voltage = value;
            break;
        case 0x04000103: // ��ŵ����
            packet.scene_data.charge.charge_discharge = value;
            break;
        case 0x00010100: // ��·��״̬
            packet.common.breaker_state = (value == 1) ? BREAKER_CLOSE : BREAKER_OPEN;
            break;
    }
    #endif
}
        dlt645_index = 0; // ���û���������
      }
    } else {
      dlt645_index = 0; // ���������������
    }
  }
  
  // �����жϣ����ڽ����ϵ磩
  if (__HAL_UART_GET_FLAG(RS485_HUART, UART_FLAG_TXE))
  {
		trip_breaker((uint8_t *)BREAKER_ADDRESS,RS485_HUART);
    // ������ɴ��� - �л��ؽ���ģʽ
    HAL_GPIO_WritePin(RS485_RE_GPIO_PORT, RS485_RE_PIN, GPIO_PIN_RESET);
    __HAL_UART_DISABLE_IT(RS485_HUART, UART_IT_TXE); // ���÷����ж�
  }
  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt / USART2 wake-up interrupt through EXTI line 26.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
if (__HAL_UART_GET_FLAG(LORA_HUART, UART_FLAG_RXNE)) {
    uint8_t data = (uint8_t)(huart2.Instance->RDR & 0xFF);
    
    // ����LoRa����
    static char lora_rx_buf[64];
    static uint8_t lora_index = 0;
    
    // ������Ч�ַ�
    if (data >= 32 && data <= 126 && lora_index < sizeof(lora_rx_buf) - 1) {
      lora_rx_buf[lora_index++] = data;
    }
    
    // ������������
    if (data == '\n' || lora_index >= sizeof(lora_rx_buf) - 1) {
      if (lora_index > 0) {
        lora_rx_buf[lora_index] = '\0';
        
        // ��������ʽ
        if (strncmp(lora_rx_buf, "CMD:", 4) == 0) {
          char* device_id = lora_rx_buf + 4;
          char* cmd = strchr(device_id, ':');
          
          if (cmd) {
            *cmd = '\0';
            cmd++;
            
            // ����Ƿ�Ϊ���豸����
            if (strcmp(device_id, DEVICE_ID) == 0) {
              // ִ������
              if (strcmp(cmd, "REBOOT") == 0) {
                NVIC_SystemReset();
              } 
              else if (strcmp(cmd, "RESET") == 0) {
                reset_breaker((uint8_t*)BREAKER_ADDRESS, RS485_HUART);
              } 
              else if (strcmp(cmd, "TRIP") == 0) {
                trip_breaker((uint8_t*)BREAKER_ADDRESS, RS485_HUART);
              }
              
              // ����ACK��Ӧ
              char ack[32];
//              snprintf(ack, sizeof(ack), "ACK:%s:DONE\r\n", DEVICE_ID);
              HAL_UART_Transmit(LORA_HUART, (uint8_t*)ack, strlen(ack), 100);
            }
          }
        }
        lora_index = 0; // ���û�����
      }
    }
  }
  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt / USART3 wake-up interrupt through EXTI line 28.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */
if (__HAL_UART_GET_FLAG(LTE_HUART, UART_FLAG_RXNE)) {
    uint8_t data = (uint8_t)(huart3.Instance->RDR & 0xFF);
    
    // ����4G����
    static char g4_rx_buf[256];
    static uint16_t g4_index = 0;
    
    // �����н����򻺳�����
    if (data == '\n' || g4_index >= sizeof(g4_rx_buf) - 1) {
      if (g4_index > 0) {
        g4_rx_buf[g4_index] = '\0';
        
        // ���MQTT��Ϣ��ʽ
        char* topic_start = strstr(g4_rx_buf, "\"topic\":\"");
        char* payload_start = strstr(g4_rx_buf, "\"payload\":");
        
        if (topic_start && payload_start) {
          // ��ȡtopic
          char* topic = topic_start + 9;
          char* topic_end = strchr(topic, '"');
          if (topic_end) *topic_end = '\0';
          
          // ��ȡpayload
          char* payload = payload_start + 10;
          char* payload_end = strrchr(payload, '}');
          if (payload_end) *(payload_end + 1) = '\0';
          
          // ����MQTT��Ϣ
          process_mqtt_message(topic, payload,LORA_HUART);
        }
        g4_index = 0; // ���û�����
      }
    } 
    // �洢��Ч�ַ�
    else if (data >= 32 && data <= 126) {
      g4_rx_buf[g4_index++] = data;
    }
  }
  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
