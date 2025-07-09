#include "mlx90614.h"
#include <stdint.h>

// SMBus 延迟函数
void SMBus_Delay(uint16_t time) {
    uint16_t i;
    for (i = 0; i < time; i++) {
        __NOP(); __NOP(); __NOP(); __NOP();
    }
}

// 设置SDA为输入模式
static void SMBUS_SDA_IN(MLX90614_HandleTypeDef* sensor) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = sensor->sda_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(sensor->sda_port, &GPIO_InitStruct);
}

// 设置SDA为开漏输出模式
static void SMBUS_SDA_OUT(MLX90614_HandleTypeDef* sensor) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = sensor->sda_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(sensor->sda_port, &GPIO_InitStruct);
}

// 初始化SMBus
void SMBus_Init(MLX90614_HandleTypeDef* sensor) {
    // 初始化SCL引脚
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = sensor->scl_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(sensor->scl_port, &GPIO_InitStruct);
		
		 // 初始化SDA引脚
    SMBUS_SDA_OUT(sensor);
    
    // 初始状态：SCL高，SDA高
    HAL_GPIO_WritePin(sensor->scl_port, sensor->scl_pin, GPIO_PIN_SET);
  
		HAL_GPIO_WritePin(sensor->sda_port, sensor->sda_pin, GPIO_PIN_SET);
}


// 生成START条件
void SMBus_StartBit(MLX90614_HandleTypeDef* sensor) {
    SMBUS_SDA_OUT(sensor);
    HAL_GPIO_WritePin(sensor->sda_port, sensor->sda_pin, GPIO_PIN_SET);
    SMBus_Delay(10);
    HAL_GPIO_WritePin(sensor->scl_port, sensor->scl_pin, GPIO_PIN_SET);
    SMBus_Delay(10);
    HAL_GPIO_WritePin(sensor->sda_port, sensor->sda_pin, GPIO_PIN_RESET);
    SMBus_Delay(10);
    HAL_GPIO_WritePin(sensor->scl_port, sensor->scl_pin, GPIO_PIN_RESET);
    SMBus_Delay(10);
}
	
// 生成STOP条件
void SMBus_StopBit(MLX90614_HandleTypeDef* sensor) {
    SMBUS_SDA_OUT(sensor);
    HAL_GPIO_WritePin(sensor->scl_port, sensor->scl_pin, GPIO_PIN_RESET);
    SMBus_Delay(10);
    HAL_GPIO_WritePin(sensor->sda_port, sensor->sda_pin, GPIO_PIN_RESET);
    SMBus_Delay(10);
    HAL_GPIO_WritePin(sensor->scl_port, sensor->scl_pin, GPIO_PIN_SET);
    SMBus_Delay(10);
    HAL_GPIO_WritePin(sensor->sda_port, sensor->sda_pin, GPIO_PIN_SET);
    SMBus_Delay(10);
}



// 发送一个位
void SMBus_SendBit(MLX90614_HandleTypeDef* sensor, uint8_t bit_out) {
    SMBUS_SDA_OUT(sensor);
    if (bit_out == 0) {
        HAL_GPIO_WritePin(sensor->sda_port, sensor->sda_pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(sensor->sda_port, sensor->sda_pin, GPIO_PIN_SET);
    }
    SMBus_Delay(4);
    HAL_GPIO_WritePin(sensor->scl_port, sensor->scl_pin, GPIO_PIN_SET);
    SMBus_Delay(12);
    HAL_GPIO_WritePin(sensor->scl_port, sensor->scl_pin, GPIO_PIN_RESET);
    SMBus_Delay(6);
}

// 发送一个字节
uint8_t SMBus_SendByte(MLX90614_HandleTypeDef* sensor, uint8_t Tx_buffer) {
    uint8_t Bit_counter;
    uint8_t bit_out;

    for (Bit_counter = 8; Bit_counter; Bit_counter--) {
        bit_out = (Tx_buffer & 0x80) ? 1 : 0;
        SMBus_SendBit(sensor, bit_out);
        Tx_buffer <<= 1;
    }

    return SMBus_ReceiveBit(sensor);
}

// 接收一个位
uint8_t SMBus_ReceiveBit(MLX90614_HandleTypeDef* sensor) {
    uint8_t Ack_bit;
    
    SMBUS_SDA_IN(sensor);
    HAL_GPIO_WritePin(sensor->sda_port, sensor->sda_pin, GPIO_PIN_SET);
    SMBus_Delay(2);
    HAL_GPIO_WritePin(sensor->scl_port, sensor->scl_pin, GPIO_PIN_SET);
    SMBus_Delay(5);
    
    Ack_bit = HAL_GPIO_ReadPin(sensor->sda_port, sensor->sda_pin);
    
    HAL_GPIO_WritePin(sensor->scl_port, sensor->scl_pin, GPIO_PIN_RESET);
    SMBus_Delay(3);
    
    return Ack_bit;
}

// 接收一个字节
uint8_t SMBus_ReceiveByte(MLX90614_HandleTypeDef* sensor, uint8_t ack_nack) {
    uint8_t RX_buffer = 0;
    uint8_t Bit_Counter;

    for (Bit_Counter = 8; Bit_Counter; Bit_Counter--) {
        RX_buffer <<= 1;
        if (SMBus_ReceiveBit(sensor)) {
            RX_buffer |= 0x01;
        }
    }
    
    SMBus_SendBit(sensor, ack_nack);
    return RX_buffer;
}

// PEC校验计算
uint8_t PEC_Calculation(uint8_t pec[])
{
    uint8_t     crc[6];
    uint8_t    BitPosition=47;
    uint8_t    shift;
    uint8_t    i;
    uint8_t    j;
    uint8_t    temp;

    do
    {
        /*Load pattern value 0x000000000107*/
        crc[5]=0;
        crc[4]=0;
        crc[3]=0;
        crc[2]=0;
        crc[1]=0x01;
        crc[0]=0x07;

        /*Set maximum bit position at 47 ( six bytes byte5...byte0,MSbit=47)*/
        BitPosition=47;

        /*Set shift position at 0*/
        shift=0;

        /*Find first "1" in the transmited message beginning from the MSByte byte5*/
        i=5;
        j=0;
        while((pec[i]&(0x80>>j))==0 && i>0)
        {
            BitPosition--;
            if(j<7)
            {
                j++;
            }
            else
            {
                j=0x00;
                i--;
            }
        }/*End of while */

        /*Get shift value for pattern value*/
        shift=BitPosition-8;

        /*Shift pattern value */
        while(shift)
        {
            for(i=5; i<0xFF; i--)
            {
                if((crc[i-1]&0x80) && (i>0))
                {
                    temp=1;
                }
                else
                {
                    temp=0;
                }
                crc[i]<<=1;
                crc[i]+=temp;
            }/*End of for*/
            shift--;
        }/*End of while*/

        /*Exclusive OR between pec and crc*/
        for(i=0; i<=5; i++)
        {
            pec[i] ^=crc[i];
        }/*End of for*/
    }
    while(BitPosition>8); /*End of do-while*/

    return pec[0];
}


// 读取内存
uint16_t SMBus_ReadMemory(MLX90614_HandleTypeDef* sensor, uint8_t slaveAddress, uint8_t command) {
    uint16_t data = 0;
    uint8_t DataL = 0, DataH = 0, Pec = 0;
    uint8_t arr[6];
    uint8_t ErrorCounter = 3; // 重试次数
    
    slaveAddress <<= 1;
    
    do {
        SMBus_StopBit(sensor);
        ErrorCounter--;
        if (!ErrorCounter) break;
        
        SMBus_StartBit(sensor);
        if (SMBus_SendByte(sensor, slaveAddress)) continue;
        if (SMBus_SendByte(sensor, command)) continue;
        
        SMBus_StartBit(sensor);
        if (SMBus_SendByte(sensor, slaveAddress | 0x01)) continue;
        
        DataL = SMBus_ReceiveByte(sensor, ACK);
        DataH = SMBus_ReceiveByte(sensor, ACK);
        Pec = SMBus_ReceiveByte(sensor, NACK);
        SMBus_StopBit(sensor);
        
        arr[5] = slaveAddress;
        arr[4] = command;
        arr[3] = slaveAddress | 0x01;
        arr[2] = DataL;
        arr[1] = DataH;
        arr[0] = 0;
    } while (PEC_Calculation(arr) != Pec);
    
    data = (DataH << 8) | DataL;
    return data;
}

// 读取温度值
float SMBus_ReadTemp(MLX90614_HandleTypeDef* sensor) {
    uint16_t raw = SMBus_ReadMemory(sensor, SA, RAM_ACCESS | RAM_TOBJ1);
    return raw * 0.02f - 273.15f;
}



