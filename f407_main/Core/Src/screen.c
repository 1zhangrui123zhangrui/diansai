#include "screen.h"

#include <stdio.h>
#include <string.h>

#define MAX_CMD_CALLBACKS 8u
#define SCREEN_OBJNAME_MAX_LEN 16u

typedef struct {
    uint8_t cmd;
    ScreenCmdCallback_t callback;
} ScreenCmdEntry_t;

typedef enum {
    SCREEN_RX_WAIT_HEAD1 = 0,
    SCREEN_RX_WAIT_HEAD2,
    SCREEN_RX_WAIT_CMD,
    SCREEN_RX_WAIT_SEQ,
} ScreenRxState_t;

static UART_HandleTypeDef *s_huart = NULL;
static uint8_t s_rx_byte = 0u;
static uint8_t s_current_page = SCREEN_PAGE_HOME;

static float s_fire_x[MAX_FIRE_COUNT] = {0.0f};
static float s_fire_y[MAX_FIRE_COUNT] = {0.0f};
static uint8_t s_fire_count = 0u;

static ScreenCmdEntry_t s_cmd_table[MAX_CMD_CALLBACKS] = {0};
static uint8_t s_cmd_count = 0u;
static ScreenSeqCallback_t s_seq_callback = NULL;

static ScreenRxState_t s_rx_state = SCREEN_RX_WAIT_HEAD1;
static uint8_t s_seq_buf[5] = {0};
static uint8_t s_seq_idx = 0u;

static const uint8_t SCREEN_END[3] = {0xFFu, 0xFFu, 0xFFu};

static void Screen_SendRaw(const char *command)
{
    if ((s_huart == NULL) || (command == NULL)) {
        return;
    }

    HAL_UART_Transmit(s_huart, (uint8_t *)command, (uint16_t)strlen(command), 100u);
    HAL_UART_Transmit(s_huart, (uint8_t *)SCREEN_END, sizeof(SCREEN_END), 100u);
}

static void Screen_DispatchCmd(uint8_t cmd)
{
    uint8_t index;

    for (index = 0u; index < s_cmd_count; index++) {
        if ((s_cmd_table[index].cmd == cmd) && (s_cmd_table[index].callback != NULL)) {
            s_cmd_table[index].callback();
            return;
        }
    }
}

static void Screen_FlushFire(void)
{
    char command[40];
    uint8_t index;

    for (index = 0u; index < MAX_FIRE_COUNT; index++) {
        if (index < s_fire_count) {
            snprintf(command, sizeof(command), "tF%uX.txt=\"%.2f\"", (unsigned)(index + 1u), s_fire_x[index]);
            Screen_SendRaw(command);
            snprintf(command, sizeof(command), "tF%uY.txt=\"%.2f\"", (unsigned)(index + 1u), s_fire_y[index]);
            Screen_SendRaw(command);
        } else {
            snprintf(command, sizeof(command), "tF%uX.txt=\"--\"", (unsigned)(index + 1u));
            Screen_SendRaw(command);
            snprintf(command, sizeof(command), "tF%uY.txt=\"--\"", (unsigned)(index + 1u));
            Screen_SendRaw(command);
        }
    }
}

static void Screen_RestartReceiveIT(void)
{
    if (s_huart != NULL) {
        HAL_UART_Receive_IT(s_huart, &s_rx_byte, 1u);
    }
}

void Screen_Init(UART_HandleTypeDef *huart)
{
    s_huart = huart;
    s_current_page = SCREEN_PAGE_HOME;
    s_fire_count = 0u;
    s_cmd_count = 0u;
    s_seq_callback = NULL;
    s_rx_state = SCREEN_RX_WAIT_HEAD1;
    s_seq_idx = 0u;

    memset(s_fire_x, 0, sizeof(s_fire_x));
    memset(s_fire_y, 0, sizeof(s_fire_y));
    memset(s_cmd_table, 0, sizeof(s_cmd_table));

    Screen_RestartReceiveIT();
}

void Screen_RegisterCallback(uint8_t cmd, ScreenCmdCallback_t callback)
{
    uint8_t index;

    for (index = 0u; index < s_cmd_count; index++) {
        if (s_cmd_table[index].cmd == cmd) {
            s_cmd_table[index].callback = callback;
            return;
        }
    }

    if (s_cmd_count < MAX_CMD_CALLBACKS) {
        s_cmd_table[s_cmd_count].cmd = cmd;
        s_cmd_table[s_cmd_count].callback = callback;
        s_cmd_count++;
    }
}

void Screen_RegisterSequenceCallback(ScreenSeqCallback_t callback)
{
    s_seq_callback = callback;
}

uint8_t Screen_GetCurrentPage(void)
{
    return s_current_page;
}

void Screen_SetCoord(float x, float y)
{
    char command[32];

    snprintf(command, sizeof(command), "tX.txt=\"%.2f\"", x);
    Screen_SendRaw(command);

    snprintf(command, sizeof(command), "tY.txt=\"%.2f\"", y);
    Screen_SendRaw(command);
}

void Screen_RecordFire(float x, float y)
{
    if (s_fire_count >= MAX_FIRE_COUNT) {
        return;
    }

    s_fire_x[s_fire_count] = x;
    s_fire_y[s_fire_count] = y;
    s_fire_count++;

    if (s_current_page == SCREEN_PAGE_FIRE) {
        Screen_FlushFire();
    }
}

void Screen_SetText(const char *objname, const char *text)
{
    char safe_text[40];
    char command[SCREEN_OBJNAME_MAX_LEN + sizeof(safe_text) + 10u];
    size_t src_index = 0u;
    size_t dst_index = 0u;

    if ((objname == NULL) || (text == NULL)) {
        return;
    }

    while ((text[src_index] != '\0') && (dst_index + 1u < sizeof(safe_text))) {
        safe_text[dst_index++] = (text[src_index] == '"') ? '\'' : text[src_index];
        src_index++;
    }
    safe_text[dst_index] = '\0';

    snprintf(command, sizeof(command), "%s.txt=\"%s\"", objname, safe_text);
    Screen_SendRaw(command);
}

void Screen_OnByteReceived(uint8_t byte)
{
    uint8_t sequence[5];
    uint8_t index;
    uint8_t valid;

    switch (s_rx_state) {
        case SCREEN_RX_WAIT_HEAD1:
            if (byte == SCREEN_FRAME_HEAD) {
                s_rx_state = SCREEN_RX_WAIT_HEAD2;
            }
            break;

        case SCREEN_RX_WAIT_HEAD2:
            s_rx_state = (byte == SCREEN_FRAME_HEAD) ? SCREEN_RX_WAIT_CMD : SCREEN_RX_WAIT_HEAD1;
            break;

        case SCREEN_RX_WAIT_CMD:
            if ((byte >= SCREEN_PAGE_REPORT_BASE) &&
                (byte < (SCREEN_PAGE_REPORT_BASE + SCREEN_PAGE_FIRE + 1u))) {
                s_current_page = (uint8_t)(byte - SCREEN_PAGE_REPORT_BASE);
                if (s_current_page == SCREEN_PAGE_FIRE) {
                    Screen_FlushFire();
                }
                s_rx_state = SCREEN_RX_WAIT_HEAD1;
            } else if (byte == CMD_SEQ_PATROL) {
                s_seq_idx = 0u;
                s_rx_state = SCREEN_RX_WAIT_SEQ;
            } else {
                Screen_DispatchCmd(byte);
                s_rx_state = SCREEN_RX_WAIT_HEAD1;
            }
            break;

        case SCREEN_RX_WAIT_SEQ:
            if (byte == SCREEN_FRAME_TAIL) {
                if ((s_seq_idx == 5u) && (s_seq_callback != NULL)) {
                    valid = 1u;
                    for (index = 0u; index < 5u; index++) {
                        if ((s_seq_buf[index] >= '1') && (s_seq_buf[index] <= '5')) {
                            sequence[index] = (uint8_t)(s_seq_buf[index] - '0');
                        } else {
                            valid = 0u;
                            break;
                        }
                    }
                    if (valid != 0u) {
                        s_seq_callback(sequence);
                    }
                }
                s_rx_state = SCREEN_RX_WAIT_HEAD1;
            } else if (s_seq_idx < 5u) {
                s_seq_buf[s_seq_idx++] = byte;
            } else {
                s_rx_state = SCREEN_RX_WAIT_HEAD1;
            }
            break;

        default:
            s_rx_state = SCREEN_RX_WAIT_HEAD1;
            break;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if ((s_huart != NULL) && (huart == s_huart)) {
        Screen_OnByteReceived(s_rx_byte);
        Screen_RestartReceiveIT();
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if ((s_huart != NULL) && (huart == s_huart)) {
        s_rx_state = SCREEN_RX_WAIT_HEAD1;
        s_seq_idx = 0u;
        Screen_RestartReceiveIT();
    }
}
