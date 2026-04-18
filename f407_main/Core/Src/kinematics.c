#include "kinematics.h"

#include <math.h>

#include "config.h"

static const AnchorPoint s_anchors[4] =
{
    {A1_X_CM, A1_Y_CM, A1_Z_CM},
    {A2_X_CM, A2_Y_CM, A2_Z_CM},
    {A3_X_CM, A3_Y_CM, A3_Z_CM},
    {A4_X_CM, A4_Y_CM, A4_Z_CM},
};

void Kinematics_CalcLengths(const Point2D *laser_pos, RopeLength *lengths)
{
    uint8_t i = 0U;
    float dx = 0.0f;
    float dy = 0.0f;
    float dz = 0.0f;

    if ((laser_pos == 0) || (lengths == 0))
    {
        return;
    }

    for (i = 0U; i < 4U; ++i)
    {
        dx = laser_pos->x - s_anchors[i].x;
        dy = laser_pos->y - s_anchors[i].y;
        dz = PLATFORM_HEIGHT_CM - s_anchors[i].z;
        lengths->value[i] = sqrtf(dx * dx + dy * dy + dz * dz);
    }
}

int32_t Kinematics_LengthDeltaToPulse(float delta_cm)
{
    if (delta_cm >= 0.0f)
    {
        return (int32_t)((delta_cm / CM_PER_PULSE) + 0.5f);
    }
    return (int32_t)((delta_cm / CM_PER_PULSE) - 0.5f);
}

HAL_StatusTypeDef Kinematics_SolvePose(const RopeLength *lengths, Point2D *pose)
{
    float a11 = 0.0f;
    float a12 = 0.0f;
    float a22 = 0.0f;
    float b1 = 0.0f;
    float b2 = 0.0f;
    float det;
    float row_a;
    float row_b;
    float row_c;
    float k0;
    float ki;
    float dz0;
    float dzi;
    uint8_t i;

    if ((lengths == NULL) || (pose == NULL))
    {
        return HAL_ERROR;
    }

    dz0 = PLATFORM_HEIGHT_CM - s_anchors[0].z;
    k0 = (s_anchors[0].x * s_anchors[0].x) +
         (s_anchors[0].y * s_anchors[0].y) +
         (dz0 * dz0);

    for (i = 1U; i < 4U; ++i)
    {
        dzi = PLATFORM_HEIGHT_CM - s_anchors[i].z;
        ki = (s_anchors[i].x * s_anchors[i].x) +
             (s_anchors[i].y * s_anchors[i].y) +
             (dzi * dzi);

        row_a = 2.0f * (s_anchors[0].x - s_anchors[i].x);
        row_b = 2.0f * (s_anchors[0].y - s_anchors[i].y);
        row_c = (lengths->value[i] * lengths->value[i]) -
                (lengths->value[0] * lengths->value[0]) -
                ki + k0;

        a11 += row_a * row_a;
        a12 += row_a * row_b;
        a22 += row_b * row_b;
        b1 += row_a * row_c;
        b2 += row_b * row_c;
    }

    det = (a11 * a22) - (a12 * a12);
    if (fabsf(det) < 1e-4f)
    {
        return HAL_ERROR;
    }

    pose->x = ((b1 * a22) - (b2 * a12)) / det;
    pose->y = ((a11 * b2) - (a12 * b1)) / det;
    return HAL_OK;
}
