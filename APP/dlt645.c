#include <dlt645.h>


extern UART_HandleTypeDef huart1, huart2, huart3;

// 计算校验和 (从起始符到数据域结束)
static uint8_t calculate_checksum(const uint8_t *data, size_t len) {
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

// 构造读取数据帧
void dlt645_build_read_frame(uint8_t *frame, const uint8_t *address, uint8_t di3, uint8_t di2, uint8_t di1, uint8_t di0) {
    // 帧起始符
    frame[0] = 0x68;

    // 地址域 (低字节在前)
    memcpy(&frame[1], address, 6);

    // 数据区起始符
    frame[7] = 0x68;

    // 控制码 (读数据)
    frame[8] = 0x11;

    // 数据域长度
    frame[9] = 0x04; // 固定4字节数据标识

    // 数据标识 (DI0-DI3)
    frame[10] = di0;
    frame[11] = di1;
    frame[12] = di2;
    frame[13] = di3;

    // 计算校验和 (从帧起始到数据域结束)
    uint8_t cs = calculate_checksum(frame, 14);
    frame[14] = cs;

    // 帧结束符
    frame[15] = 0x16;
}

// RS485控制引脚（根据实际硬件修改）
#define RS485_RE_PIN GET_PIN(B, 5)

// 构造控制帧（带密码和操作者代码）
void dlt645_build_control_frame(uint8_t *frame, const uint8_t *address,
                               uint8_t di3, uint8_t di2, uint8_t di1, uint8_t di0) {
    // 帧起始符
    frame[0] = 0x68;

    // 地址域 (低字节在前)
    memcpy(&frame[1], address, 6);

    // 数据区起始符
    frame[7] = 0x68;

    // 控制码 (写数据)
    frame[8] = 0x14; // 控制码 0x14 = 写数据

    // 数据域长度
    frame[9] = 0x0C; // 4(标识码) + 4(密码) + 4(操作者代码)

    // 数据标识 (DI0-DI3)
    frame[10] = di0;
    frame[11] = di1;
    frame[12] = di2;
    frame[13] = di3;

    // 密码 (默认00000000)
    frame[14] = 0x00; // PA
    frame[15] = 0x00; // P0
    frame[16] = 0x00; // P1
    frame[17] = 0x00; // P2

    // 操作者代码 (默认00000000)
    frame[18] = 0x00; // C0
    frame[19] = 0x00; // C1
    frame[20] = 0x00; // C2
    frame[21] = 0x00; // C3

    // 计算校验和
    uint8_t cs = calculate_checksum(frame, 22);
    frame[22] = cs;

    // 帧结束符
    frame[23] = 0x16;
}

// 发送DL/T645帧
void send_dlt645_frame(uint8_t *frame, size_t len) {
 // 设置RS485为发送模式
    HAL_GPIO_WritePin(RS485_RE_PIN_GPIO_PORT, RS485_RE_PIN_GPIO_PIN, GPIO_PIN_SET);
    HAL_Delay(1);  // 确保方向切换稳定

    // 发送数据
    HAL_UART_Transmit(&huart1, frame, len,200); 
	
     // 切换回接收模式
    HAL_Delay(1);  // 确保数据发送完成
    HAL_GPIO_WritePin(RS485_RE_PIN_GPIO_PORT, RS485_RE_PIN_GPIO_PIN, GPIO_PIN_RESET);
}

// 解析响应帧
int dlt645_parse_response(const uint8_t *data, size_t len, float *value) {
    // 基本帧校验
    if (len < 12 || data[0] != 0x68 || data[7] != 0x68) {
        return -1; // 无效帧
    }

    // 校验控制码 (正常响应)
    if (data[8] != 0x91 && data[8] != 0xB1) {
        return -2; // 非正常响应
    }

    // 数据域长度
    uint8_t data_len = data[9];
    if (data_len < 4 || len < 11 + data_len) {
        return -3; // 数据长度错误
    }

    // 校验和验证
    uint8_t calc_cs = calculate_checksum(data, 10 + data_len);
    if (calc_cs != data[10 + data_len]) {
        return -4; // 校验和错误
    }

    // 数据域处理 (减33H解码)
    uint8_t decoded[6] = {0};
    uint8_t data_start = 10;
    for (int i = 0; i < data_len; i++) {
        decoded[i] = data[data_start + i] - 0x33;
    }

    // 根据数据长度解析值
    if (data_len == 4) { // 2字节BCD格式 (XXX.X)
        uint16_t raw = (decoded[2] << 8) | decoded[3];
        *value = (raw / 10.0f);
        return 0;
    }
    else if (data_len == 5) { // 3字节BCD格式 (XXXXX.X)
        uint32_t raw = (decoded[4] << 16) | (decoded[3] << 8) | decoded[2];
        *value = (raw / 10.0f);
        return 0;
    }

    return -5; // 不支持的数据格式
}
