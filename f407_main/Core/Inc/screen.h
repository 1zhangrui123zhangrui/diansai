#ifndef __SCREEN_H
#define __SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>

#include "protocol.h"

typedef void (*ScreenCmdCallback_t)(void);
typedef void (*ScreenSeqCallback_t)(uint8_t *seq);

void Screen_Init(UART_HandleTypeDef *huart);
void Screen_RegisterCallback(uint8_t cmd, ScreenCmdCallback_t callback);
void Screen_RegisterSequenceCallback(ScreenSeqCallback_t callback);
uint8_t Screen_GetCurrentPage(void);
void Screen_SetCoord(float x, float y);
void Screen_RecordFire(float x, float y);
void Screen_SetText(const char *objname, const char *text);
void Screen_OnByteReceived(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* __SCREEN_H */
