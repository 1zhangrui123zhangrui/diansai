#include "app_control.h"

#include <string.h>

#include "config.h"
#include "fire_link.h"
#include "homing.h"
#include "motion_sync.h"

static AppState s_app_state = APP_STATE_BOOT_HOME;
static AppState s_resume_state = APP_STATE_IDLE;
static Point2D s_manual_points[5];
static Point2D s_fire_points[FIRE_RECORD_MAX];
static uint8_t s_manual_count = 0U;
static uint8_t s_manual_index = 0U;
static uint8_t s_patrol_index = 0U;
static uint8_t s_fire_count = 0U;

static const Point2D s_patrol_points[4] =
{
    {PATROL_P1_X_CM, PATROL_P1_Y_CM},
    {PATROL_P2_X_CM, PATROL_P2_Y_CM},
    {PATROL_P3_X_CM, PATROL_P3_Y_CM},
    {PATROL_P4_X_CM, PATROL_P4_Y_CM},
};

static void AppControl_RecordFirePoint(Point2D pose)
{
    if (s_fire_count < FIRE_RECORD_MAX)
    {
        s_fire_points[s_fire_count++] = pose;
    }
}

static void AppControl_HandleFireEvent(void)
{
    Point2D pose;

    if ((FireLink_HasNewEvent() == 0U) ||
        (s_app_state == APP_STATE_BOOT_HOME) ||
        (s_app_state == APP_STATE_FIRE_HOLD))
    {
        return;
    }

    s_resume_state = s_app_state;

    if (MotionSync_IsBusy() != 0U)
    {
        if ((s_app_state == APP_STATE_EDGE_PATROL) && (s_patrol_index > 0U))
        {
            s_patrol_index--;
        }
        else if ((s_app_state == APP_STATE_MANUAL_SEQUENCE) && (s_manual_index > 0U))
        {
            s_manual_index--;
        }
    }

    (void)MotionSync_AbortAndRefresh();
    pose = MotionSync_GetCurrentPose();
    AppControl_RecordFirePoint(pose);
    FireLink_ClearEvent();
    s_app_state = APP_STATE_FIRE_HOLD;
}

void AppControl_Init(void)
{
    memset(s_manual_points, 0, sizeof(s_manual_points));
    memset(s_fire_points, 0, sizeof(s_fire_points));
    s_manual_count = 0U;
    s_manual_index = 0U;
    s_patrol_index = 0U;
    s_fire_count = 0U;
    s_app_state = APP_STATE_BOOT_HOME;
    s_resume_state = APP_STATE_IDLE;

    MotionSync_Init();
    Homing_Init();
    Homing_Start();
}

void AppControl_RequestReset(void)
{
    if (s_app_state == APP_STATE_IDLE)
    {
        s_app_state = APP_STATE_RESET;
    }
}

void AppControl_RequestEdgePatrol(void)
{
    if (s_app_state == APP_STATE_IDLE)
    {
        s_patrol_index = 0U;
        s_app_state = APP_STATE_EDGE_PATROL;
    }
}

void AppControl_LoadManualTargets(const Point2D *points, uint8_t count)
{
    if ((points == NULL) || (count == 0U) || (count > 5U))
    {
        return;
    }

    memcpy(s_manual_points, points, sizeof(Point2D) * count);
    s_manual_count = count;
    s_manual_index = 0U;

    if (s_app_state == APP_STATE_IDLE)
    {
        s_app_state = APP_STATE_MANUAL_SEQUENCE;
    }
}

void AppControl_Task(uint32_t now_ms)
{
    Point2D reset_point = {0.0f, 0.0f};

    MotionSync_Task(now_ms);
    FireLink_Task(now_ms);
    AppControl_HandleFireEvent();

    switch (s_app_state)
    {
    case APP_STATE_BOOT_HOME:
        Homing_Task(now_ms);
        if (Homing_IsDone() != 0U)
        {
            s_app_state = APP_STATE_IDLE;
        }
        break;

    case APP_STATE_IDLE:
        break;

    case APP_STATE_RESET:
        if ((MotionSync_IsBusy() == 0U) && (MotionSync_MoveTo(&reset_point) == HAL_OK))
        {
            s_app_state = APP_STATE_IDLE;
        }
        break;

    case APP_STATE_EDGE_PATROL:
        if (MotionSync_IsBusy() == 0U)
        {
            if (s_patrol_index >= 4U)
            {
                s_app_state = APP_STATE_IDLE;
            }
            else if (MotionSync_MoveTo(&s_patrol_points[s_patrol_index]) == HAL_OK)
            {
                s_patrol_index++;
            }
        }
        break;

    case APP_STATE_MANUAL_SEQUENCE:
        if (MotionSync_IsBusy() == 0U)
        {
            if (s_manual_index >= s_manual_count)
            {
                s_app_state = APP_STATE_IDLE;
            }
            else if (MotionSync_MoveTo(&s_manual_points[s_manual_index]) == HAL_OK)
            {
                s_manual_index++;
            }
        }
        break;

    case APP_STATE_FIRE_HOLD:
        if (FireLink_IsActive() == 0U)
        {
            s_app_state = s_resume_state;
        }
        break;

    default:
        s_app_state = APP_STATE_IDLE;
        break;
    }
}

AppState AppControl_GetState(void)
{
    return s_app_state;
}

uint8_t AppControl_GetFireCount(void)
{
    return s_fire_count;
}

uint8_t AppControl_GetFirePoint(uint8_t index, Point2D *point)
{
    if ((point == NULL) || (index >= s_fire_count))
    {
        return 0U;
    }

    *point = s_fire_points[index];
    return 1U;
}
