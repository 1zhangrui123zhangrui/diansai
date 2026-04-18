#ifndef ABS_ENCODER_H
#define ABS_ENCODER_H

#include "stm32f4xx_hal.h"
#include "types.h"

void AbsEncoder_Init(void);
HAL_StatusTypeDef AbsEncoder_ReadAngleDeg(MotorId id, float *angle_deg);
HAL_StatusTypeDef AbsEncoder_ReadAllAngles(float angle_deg[4]);
HAL_StatusTypeDef AbsEncoder_ReadRopeLengths(RopeLength *lengths);

#endif
