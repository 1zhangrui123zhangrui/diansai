#ifndef MOTION_SYNC_H
#define MOTION_SYNC_H

#include "stm32f4xx_hal.h"
#include "types.h"

void MotionSync_Init(void);
void MotionSync_Task(uint32_t now_ms);
void MotionSync_SetCurrentPose(float x_cm, float y_cm);
void MotionSync_SetMeasuredState(const Point2D *pose, const RopeLength *lengths);
HAL_StatusTypeDef MotionSync_RefreshFeedback(void);
HAL_StatusTypeDef MotionSync_AbortAndRefresh(void);
Point2D MotionSync_GetCurrentPose(void);
HAL_StatusTypeDef MotionSync_MoveTo(const Point2D *target);
HAL_StatusTypeDef MotionSync_MoveBy(float dx_cm, float dy_cm);
uint8_t MotionSync_IsBusy(void);

#endif
