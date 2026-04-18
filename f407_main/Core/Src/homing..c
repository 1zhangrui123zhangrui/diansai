#include "homing.h"

#include <math.h>

#include "config.h"
#include "motion_sync.h"

static HomeState s_home_state = HOME_STATE_IDLE;
static uint8_t s_home_retry = 0U;

void Homing_Init(void)
{
    s_home_state = HOME_STATE_IDLE;
    s_home_retry = 0U;
}

void Homing_Start(void)
{
    s_home_state = HOME_STATE_SYNC_ENCODER;
    s_home_retry = 0U;
}

void Homing_Task(uint32_t now_ms)
{
    Point2D center = {0.0f, 0.0f};
    Point2D pose;
    (void)now_ms;

    MotionSync_Task(now_ms);

    switch (s_home_state)
    {
    case HOME_STATE_IDLE:
        break;

    case HOME_STATE_SYNC_ENCODER:
        if (MotionSync_RefreshFeedback() == HAL_OK)
        {
            s_home_state = HOME_STATE_SOLVE_POSE;
        }
        else if (s_home_retry++ >= HOME_RETRY_MAX)
        {
            s_home_state = HOME_STATE_FAIL;
        }
        break;

    case HOME_STATE_SOLVE_POSE:
        pose = MotionSync_GetCurrentPose();
        if ((fabsf(pose.x) <= HOME_CENTER_TOL_CM) &&
            (fabsf(pose.y) <= HOME_CENTER_TOL_CM))
        {
            s_home_state = HOME_STATE_DONE;
        }
        else if (MotionSync_MoveTo(&center) == HAL_OK)
        {
            s_home_state = HOME_STATE_MOVE_CENTER;
        }
        else if (s_home_retry++ >= HOME_RETRY_MAX)
        {
            s_home_state = HOME_STATE_FAIL;
        }
        break;

    case HOME_STATE_MOVE_CENTER:
        if (MotionSync_IsBusy() == 0U)
        {
            if (MotionSync_RefreshFeedback() == HAL_OK)
            {
                pose = MotionSync_GetCurrentPose();
                if ((fabsf(pose.x) <= HOME_CENTER_TOL_CM) &&
                    (fabsf(pose.y) <= HOME_CENTER_TOL_CM))
                {
                    s_home_state = HOME_STATE_DONE;
                }
                else if (s_home_retry++ < HOME_RETRY_MAX)
                {
                    s_home_state = HOME_STATE_SOLVE_POSE;
                }
                else
                {
                    s_home_state = HOME_STATE_FAIL;
                }
            }
            else if (s_home_retry++ >= HOME_RETRY_MAX)
            {
                s_home_state = HOME_STATE_FAIL;
            }
        }
        break;

    case HOME_STATE_DONE:
    case HOME_STATE_FAIL:
    default:
        break;
    }
}

HomeState Homing_GetState(void)
{
    return s_home_state;
}

uint8_t Homing_IsDone(void)
{
    return s_home_state == HOME_STATE_DONE;
}
