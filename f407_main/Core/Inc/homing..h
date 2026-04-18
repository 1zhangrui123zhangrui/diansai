#ifndef HOMING_H
#define HOMING_H

#include <stdint.h>
#include "types.h"

void Homing_Init(void);
void Homing_Start(void);
void Homing_Task(uint32_t now_ms);
HomeState Homing_GetState(void);
uint8_t Homing_IsDone(void);

#endif
