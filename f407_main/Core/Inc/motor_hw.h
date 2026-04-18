#ifndef MOTOR_HW_H
#define MOTOR_HW_H

#include "stm32f4xx_hal.h"
#include "types.h"

void MotorHw_Init(void);
HAL_StatusTypeDef MotorHw_StartRelative(const int32_t signed_pulses[4], uint32_t base_rate_hz);
void MotorHw_Stop(MotorId id);
void MotorHw_StopAll(void);
uint8_t MotorHw_IsBusy(MotorId id);
uint8_t MotorHw_AnyBusy(void);
void MotorHw_TimerElapsedCallback(TIM_HandleTypeDef *htim);

#endif
