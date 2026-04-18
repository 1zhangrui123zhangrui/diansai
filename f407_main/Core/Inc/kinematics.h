#ifndef KINEMATICS_H
#define KINEMATICS_H

#include "stm32f4xx_hal.h"
#include "types.h"

void Kinematics_CalcLengths(const Point2D *laser_pos, RopeLength *lengths);
int32_t Kinematics_LengthDeltaToPulse(float delta_cm);
HAL_StatusTypeDef Kinematics_SolvePose(const RopeLength *lengths, Point2D *pose);

#endif
