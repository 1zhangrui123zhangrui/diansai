#ifndef FIRE_LINK_H
#define FIRE_LINK_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

void FireLink_Init(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState active_state, uint32_t debounce_ms);
void FireLink_Task(uint32_t now_ms);
uint8_t FireLink_IsActive(void);
uint8_t FireLink_HasNewEvent(void);
void FireLink_ClearEvent(void);

#endif
