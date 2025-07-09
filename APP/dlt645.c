#include <dlt645.h>


extern UART_HandleTypeDef huart1, huart2, huart3;

// ����У��� (����ʼ�������������)
static uint8_t calculate_checksum(const uint8_t *data, size_t len) {
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

// �����ȡ����֡
void dlt645_build_read_frame(uint8_t *frame, const uint8_t *address, uint8_t di3, uint8_t di2, uint8_t di1, uint8_t di0) {
    // ֡��ʼ��
    frame[0] = 0x68;

    // ��ַ�� (���ֽ���ǰ)
    memcpy(&frame[1], address, 6);

    // ��������ʼ��
    frame[7] = 0x68;

    // ������ (������)
    frame[8] = 0x11;

    // �����򳤶�
    frame[9] = 0x04; // �̶�4�ֽ����ݱ�ʶ

    // ���ݱ�ʶ (DI0-DI3)
    frame[10] = di0;
    frame[11] = di1;
    frame[12] = di2;
    frame[13] = di3;

    // ����У��� (��֡��ʼ�����������)
    uint8_t cs = calculate_checksum(frame, 14);
    frame[14] = cs;

    // ֡������
    frame[15] = 0x16;
}

// RS485�������ţ�����ʵ��Ӳ���޸ģ�
#define RS485_RE_PIN GET_PIN(B, 5)

// �������֡��������Ͳ����ߴ��룩
void dlt645_build_control_frame(uint8_t *frame, const uint8_t *address,
                               uint8_t di3, uint8_t di2, uint8_t di1, uint8_t di0) {
    // ֡��ʼ��
    frame[0] = 0x68;

    // ��ַ�� (���ֽ���ǰ)
    memcpy(&frame[1], address, 6);

    // ��������ʼ��
    frame[7] = 0x68;

    // ������ (д����)
    frame[8] = 0x14; // ������ 0x14 = д����

    // �����򳤶�
    frame[9] = 0x0C; // 4(��ʶ��) + 4(����) + 4(�����ߴ���)

    // ���ݱ�ʶ (DI0-DI3)
    frame[10] = di0;
    frame[11] = di1;
    frame[12] = di2;
    frame[13] = di3;

    // ���� (Ĭ��00000000)
    frame[14] = 0x00; // PA
    frame[15] = 0x00; // P0
    frame[16] = 0x00; // P1
    frame[17] = 0x00; // P2

    // �����ߴ��� (Ĭ��00000000)
    frame[18] = 0x00; // C0
    frame[19] = 0x00; // C1
    frame[20] = 0x00; // C2
    frame[21] = 0x00; // C3

    // ����У���
    uint8_t cs = calculate_checksum(frame, 22);
    frame[22] = cs;

    // ֡������
    frame[23] = 0x16;
}

// ����DL/T645֡
void send_dlt645_frame(uint8_t *frame, size_t len) {
 // ����RS485Ϊ����ģʽ
    HAL_GPIO_WritePin(RS485_RE_PIN_GPIO_PORT, RS485_RE_PIN_GPIO_PIN, GPIO_PIN_SET);
    HAL_Delay(1);  // ȷ�������л��ȶ�

    // ��������
    HAL_UART_Transmit(&huart1, frame, len,200); 
	
     // �л��ؽ���ģʽ
    HAL_Delay(1);  // ȷ�����ݷ������
    HAL_GPIO_WritePin(RS485_RE_PIN_GPIO_PORT, RS485_RE_PIN_GPIO_PIN, GPIO_PIN_RESET);
}

// ������Ӧ֡
int dlt645_parse_response(const uint8_t *data, size_t len, float *value) {
    // ����֡У��
    if (len < 12 || data[0] != 0x68 || data[7] != 0x68) {
        return -1; // ��Ч֡
    }

    // У������� (������Ӧ)
    if (data[8] != 0x91 && data[8] != 0xB1) {
        return -2; // ��������Ӧ
    }

    // �����򳤶�
    uint8_t data_len = data[9];
    if (data_len < 4 || len < 11 + data_len) {
        return -3; // ���ݳ��ȴ���
    }

    // У�����֤
    uint8_t calc_cs = calculate_checksum(data, 10 + data_len);
    if (calc_cs != data[10 + data_len]) {
        return -4; // У��ʹ���
    }

    // �������� (��33H����)
    uint8_t decoded[6] = {0};
    uint8_t data_start = 10;
    for (int i = 0; i < data_len; i++) {
        decoded[i] = data[data_start + i] - 0x33;
    }

    // �������ݳ��Ƚ���ֵ
    if (data_len == 4) { // 2�ֽ�BCD��ʽ (XXX.X)
        uint16_t raw = (decoded[2] << 8) | decoded[3];
        *value = (raw / 10.0f);
        return 0;
    }
    else if (data_len == 5) { // 3�ֽ�BCD��ʽ (XXXXX.X)
        uint32_t raw = (decoded[4] << 16) | (decoded[3] << 8) | decoded[2];
        *value = (raw / 10.0f);
        return 0;
    }

    return -5; // ��֧�ֵ����ݸ�ʽ
}
