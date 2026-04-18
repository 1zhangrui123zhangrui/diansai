#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef struct
{
    float x;
    float y;
} Point2D;

typedef struct
{
    float x;
    float y;
    float z;
} AnchorPoint;

typedef struct
{
    float value[4];
} RopeLength;

typedef enum
{
    MOTOR_1 = 0,
    MOTOR_2,
    MOTOR_3,
    MOTOR_4
} MotorId;

typedef enum
{
    MOTOR_DIR_OUT = 0,
    MOTOR_DIR_IN = 1
} MotorDir;

typedef enum
{
    VISION_STATUS_NONE = 0,
    VISION_STATUS_TARGET_FOUND = 1,
    VISION_STATUS_TARGET_LOCK = 2
} VisionStatus;

typedef struct
{
    VisionStatus status;
    uint8_t blob_count;
    int16_t ref_cx;
    int16_t ref_cy;
    int16_t dx;
    int16_t dy;
    uint32_t tick_ms;
} VisionPacket;

typedef enum
{
    HOME_STATE_IDLE = 0,
    HOME_STATE_SYNC_ENCODER,
    HOME_STATE_SOLVE_POSE,
    HOME_STATE_MOVE_CENTER,
    HOME_STATE_DONE,
    HOME_STATE_FAIL
} HomeState;

typedef enum
{
    APP_STATE_BOOT_HOME = 0,
    APP_STATE_IDLE,
    APP_STATE_RESET,
    APP_STATE_EDGE_PATROL,
    APP_STATE_MANUAL_SEQUENCE,
    APP_STATE_FIRE_HOLD
} AppState;

#endif
