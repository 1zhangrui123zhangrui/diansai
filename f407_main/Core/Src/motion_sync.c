#include "motion_sync.h"

#include <string.h>

#include "abs_encoder.h"
#include "config.h"
#include "kinematics.h"
#include "motor_hw.h"

static Point2D s_current_pose;
static Point2D s_target_pose;
static RopeLength s_current_lengths;
static RopeLength s_target_lengths;
static uint8_t s_motion_busy = 0U;
static uint32_t s_last_feedback_tick = 0U;

void MotionSync_Init(void)
{
    memset(&s_current_pose, 0, sizeof(s_current_pose));
    memset(&s_target_pose, 0, sizeof(s_target_pose));
    memset(&s_current_lengths, 0, sizeof(s_current_lengths));
    memset(&s_target_lengths, 0, sizeof(s_target_lengths));
    s_motion_busy = 0U;
    s_last_feedback_tick = 0U;
}

void MotionSync_SetCurrentPose(float x_cm, float y_cm)
{
    s_current_pose.x = x_cm;
    s_current_pose.y = y_cm;
    s_target_pose = s_current_pose;
    Kinematics_CalcLengths(&s_current_pose, &s_current_lengths);
    s_target_lengths = s_current_lengths;
    s_motion_busy = 0U;
}

Point2D MotionSync_GetCurrentPose(void)
{
    return s_current_pose;
}

void MotionSync_SetMeasuredState(const Point2D *pose, const RopeLength *lengths)
{
    if ((pose == NULL) || (lengths == NULL))
    {
        return;
    }

    s_current_pose = *pose;
    s_current_lengths = *lengths;
    if (s_motion_busy == 0U)
    {
        s_target_pose = s_current_pose;
        s_target_lengths = s_current_lengths;
    }
}

HAL_StatusTypeDef MotionSync_RefreshFeedback(void)
{
    RopeLength lengths;
    Point2D pose;

    if (AbsEncoder_ReadRopeLengths(&lengths) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (Kinematics_SolvePose(&lengths, &pose) != HAL_OK)
    {
        return HAL_ERROR;
    }

    MotionSync_SetMeasuredState(&pose, &lengths);
    return HAL_OK;
}

HAL_StatusTypeDef MotionSync_MoveTo(const Point2D *target)
{
    int32_t pulses[4];
    uint8_t i = 0U;
    HAL_StatusTypeDef ret;

    if (target == NULL)
    {
        return HAL_ERROR;
    }

    if ((s_motion_busy != 0U) || (MotorHw_AnyBusy() != 0U))
    {
        return HAL_BUSY;
    }

    Kinematics_CalcLengths(target, &s_target_lengths);

    for (i = 0U; i < 4U; ++i)
    {
        pulses[i] = Kinematics_LengthDeltaToPulse(s_current_lengths.value[i] - s_target_lengths.value[i]);
    }

    ret = MotorHw_StartRelative(pulses, MAX_MOTOR_PULSE_RATE_HZ);
    if (ret == HAL_OK)
    {
        s_target_pose = *target;
        s_motion_busy = 1U;
    }
    return ret;
}

HAL_StatusTypeDef MotionSync_MoveBy(float dx_cm, float dy_cm)
{
    Point2D target = s_current_pose;
    target.x += dx_cm;
    target.y += dy_cm;
    return MotionSync_MoveTo(&target);
}

void MotionSync_Task(uint32_t now_ms)
{
    if ((s_motion_busy != 0U) && (MotorHw_AnyBusy() == 0U))
    {
        s_current_pose = s_target_pose;
        s_current_lengths = s_target_lengths;
        s_motion_busy = 0U;
        MotionSync_RefreshFeedback();
        s_last_feedback_tick = now_ms;
        return;
    }

    if (((now_ms - s_last_feedback_tick) >= MOTION_FEEDBACK_PERIOD_MS) &&
        (MotorHw_AnyBusy() == 0U))
    {
        if (MotionSync_RefreshFeedback() == HAL_OK)
        {
            s_last_feedback_tick = now_ms;
        }
    }
}

uint8_t MotionSync_IsBusy(void)
{
    return s_motion_busy;
}
