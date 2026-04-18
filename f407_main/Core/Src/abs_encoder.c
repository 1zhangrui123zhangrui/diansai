#include "abs_encoder.h"

#include "main.h"

#include <string.h>

#include "config.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;

/*
 * 鏈枃浠堕粯璁ゆ杩涢┍鍔ㄥ櫒鐨� TTL 涓插彛鍗忚涓庡巶瀹� Modbus-RTU 璇诲瘎瀛樺櫒鍗忚涓€鑷达紝
 * 閫氳繃璇诲彇 0x0046 璧峰鐨� 3 涓瘎瀛樺櫒鑾峰彇鈥滃疄鏃朵綅缃搴︹€濄€�
 * 濡傛灉浣犳墜涓婄殑 TTL 鐗堟湰鍗忚甯ф牸寮忎笉鍚岋紝鍙渶瑕佷慨鏀规湰鏂囦欢涓殑鏌ヨ涓庤В鏋愰€昏緫锛�
 * 涓婂眰鐨勭怀闀挎崲绠椼€佸潗鏍囪В绠椾笌鍥炰腑蹇冩祦绋嬮兘鏃犻渶鏀瑰姩銆�
 */

typedef struct
{
    UART_HandleTypeDef *huart;
    uint8_t addr;
    float ref_angle_deg;
    float ref_rope_cm;
    float angle_to_len_sign;
} AbsEncoderChannel;

static AbsEncoderChannel s_encoder_channels[4] =
{
    {&huart1, M1_TTL_ADDR, M1_REF_ANGLE_DEG, M1_REF_ROPE_CM, M1_ANGLE_TO_LEN_SIGN},
    {&huart2, M2_TTL_ADDR, M2_REF_ANGLE_DEG, M2_REF_ROPE_CM, M2_ANGLE_TO_LEN_SIGN},
    {&huart4, M3_TTL_ADDR, M3_REF_ANGLE_DEG, M3_REF_ROPE_CM, M3_ANGLE_TO_LEN_SIGN},
    {&huart5, M4_TTL_ADDR, M4_REF_ANGLE_DEG, M4_REF_ROPE_CM, M4_ANGLE_TO_LEN_SIGN},
};

static uint16_t AbsEncoder_ModbusCrc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFFU;
    uint16_t i = 0U;
    uint8_t j = 0U;

    for (i = 0U; i < len; ++i)
    {
        crc ^= data[i];
        for (j = 0U; j < 8U; ++j)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc >>= 1U;
                crc ^= 0xA001U;
            }
            else
            {
                crc >>= 1U;
            }
        }
    }
    return crc;
}

static HAL_StatusTypeDef AbsEncoder_QueryRealtimeAngle(AbsEncoderChannel *channel, float *angle_deg)
{
    uint8_t tx_buf[8];
    uint8_t rx_buf[11];
    uint16_t crc;
    uint16_t rx_crc;
    uint16_t sign_word;
    uint32_t magnitude;
    int32_t signed_value;

    if ((channel == NULL) || (angle_deg == NULL))
    {
        return HAL_ERROR;
    }

    tx_buf[0] = channel->addr;
    tx_buf[1] = 0x03U;
    tx_buf[2] = 0x00U;
    tx_buf[3] = 0x46U;
    tx_buf[4] = 0x00U;
    tx_buf[5] = 0x03U;
    crc = AbsEncoder_ModbusCrc16(tx_buf, 6U);
    tx_buf[6] = (uint8_t)(crc & 0xFFU);
    tx_buf[7] = (uint8_t)((crc >> 8) & 0xFFU);

    __HAL_UART_FLUSH_DRREGISTER(channel->huart);
    if (HAL_UART_Transmit(channel->huart, tx_buf, sizeof(tx_buf), ABS_ENCODER_TIMEOUT_MS) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_UART_Receive(channel->huart, rx_buf, sizeof(rx_buf), ABS_ENCODER_TIMEOUT_MS) != HAL_OK)
    {
        return HAL_ERROR;
    }

    rx_crc = (uint16_t)rx_buf[9] | ((uint16_t)rx_buf[10] << 8);
    if ((rx_buf[0] != channel->addr) || (rx_buf[1] != 0x03U) || (rx_buf[2] != 0x06U))
    {
        return HAL_ERROR;
    }
    if (AbsEncoder_ModbusCrc16(rx_buf, 9U) != rx_crc)
    {
        return HAL_ERROR;
    }

    sign_word = (uint16_t)((uint16_t)rx_buf[3] << 8) | rx_buf[4];
    magnitude = ((uint32_t)rx_buf[5] << 24) |
                ((uint32_t)rx_buf[6] << 16) |
                ((uint32_t)rx_buf[7] << 8) |
                ((uint32_t)rx_buf[8]);

    signed_value = (int32_t)magnitude;
    if (sign_word != 0U)
    {
        signed_value = -signed_value;
    }

    *angle_deg = ((float)signed_value) * 0.1f;
    return HAL_OK;
}

void AbsEncoder_Init(void)
{
    /* TTL 鏌ヨ閲囩敤闃诲寮忥紝鏃犻渶棰濆鍒濆鍖栨祦绋� */
}

HAL_StatusTypeDef AbsEncoder_ReadAngleDeg(MotorId id, float *angle_deg)
{
    uint8_t retry = 0U;

    if (id > MOTOR_4)
    {
        return HAL_ERROR;
    }

    while (retry < ABS_ENCODER_RETRY_MAX)
    {
        if (AbsEncoder_QueryRealtimeAngle(&s_encoder_channels[id], angle_deg) == HAL_OK)
        {
            return HAL_OK;
        }
        retry++;
    }

    return HAL_ERROR;
}

HAL_StatusTypeDef AbsEncoder_ReadAllAngles(float angle_deg[4])
{
    uint8_t i = 0U;

    if (angle_deg == NULL)
    {
        return HAL_ERROR;
    }

    for (i = 0U; i < 4U; ++i)
    {
        if (AbsEncoder_ReadAngleDeg((MotorId)i, &angle_deg[i]) != HAL_OK)
        {
            return HAL_ERROR;
        }
    }
    return HAL_OK;
}

HAL_StatusTypeDef AbsEncoder_ReadRopeLengths(RopeLength *lengths)
{
    float angle_deg[4];
    float delta_turn;
    uint8_t i = 0U;

    if (lengths == NULL)
    {
        return HAL_ERROR;
    }

    if (AbsEncoder_ReadAllAngles(angle_deg) != HAL_OK)
    {
        return HAL_ERROR;
    }

    for (i = 0U; i < 4U; ++i)
    {
        delta_turn = (angle_deg[i] - s_encoder_channels[i].ref_angle_deg) / 360.0f;
        lengths->value[i] = s_encoder_channels[i].ref_rope_cm +
                            s_encoder_channels[i].angle_to_len_sign *
                            delta_turn *
                            SPOOL_CIRCUMFERENCE_CM *
                            MOTOR_TO_SPOOL_RATIO;
    }

    return HAL_OK;
}
