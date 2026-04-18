#include "motor_hw.h"

#include <stdlib.h>

#include "config.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;

typedef struct
{
    GPIO_TypeDef *step_port;
    uint16_t step_pin;
    GPIO_TypeDef *dir_port;
    uint16_t dir_pin;
    TIM_HandleTypeDef *htim;
    uint8_t step_level;
    uint8_t running;
    uint32_t target_pulses;
    uint32_t current_pulses;
} MotorChannel;

static MotorChannel s_motors[4] =
{
    {GPIOE, GPIO_PIN_8,  GPIOE, GPIO_PIN_9,  &htim2, 0U, 0U, 0U, 0U},
    {GPIOE, GPIO_PIN_10, GPIOE, GPIO_PIN_11, &htim3, 0U, 0U, 0U, 0U},
    {GPIOE, GPIO_PIN_12, GPIOE, GPIO_PIN_13, &htim4, 0U, 0U, 0U, 0U},
    {GPIOE, GPIO_PIN_14, GPIOE, GPIO_PIN_15, &htim5, 0U, 0U, 0U, 0U},
};

static uint32_t MotorHw_CalcArrFromRate(uint32_t pulse_rate_hz)
{
    uint32_t toggle_hz;

    if (pulse_rate_hz < MIN_MOTOR_PULSE_RATE_HZ)
    {
        pulse_rate_hz = MIN_MOTOR_PULSE_RATE_HZ;
    }
    if (pulse_rate_hz > MAX_MOTOR_PULSE_RATE_HZ)
    {
        pulse_rate_hz = MAX_MOTOR_PULSE_RATE_HZ;
    }

    toggle_hz = pulse_rate_hz * 2U;
    if (toggle_hz == 0U)
    {
        toggle_hz = MIN_MOTOR_PULSE_RATE_HZ * 2U;
    }

    return (1000000U / toggle_hz) - 1U;
}

static HAL_StatusTypeDef MotorHw_StartSingle(MotorId id, MotorDir dir, uint32_t pulse_count, uint32_t pulse_rate_hz)
{
    MotorChannel *motor;

    if (id > MOTOR_4)
    {
        return HAL_ERROR;
    }

    motor = &s_motors[id];
    if ((pulse_count == 0U) || (motor->running != 0U))
    {
        return HAL_OK;
    }

    HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, (dir == MOTOR_DIR_IN) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(motor->step_port, motor->step_pin, GPIO_PIN_RESET);
    motor->step_level = 0U;
    motor->current_pulses = 0U;
    motor->target_pulses = pulse_count;
    motor->running = 1U;

    __HAL_TIM_SET_COUNTER(motor->htim, 0U);
    __HAL_TIM_SET_AUTORELOAD(motor->htim, MotorHw_CalcArrFromRate(pulse_rate_hz));
    HAL_TIM_Base_Start_IT(motor->htim);

    return HAL_OK;
}

void MotorHw_Init(void)
{
    uint8_t i = 0;
    for (i = 0; i < 4U; ++i)
    {
        HAL_GPIO_WritePin(s_motors[i].step_port, s_motors[i].step_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(s_motors[i].dir_port, s_motors[i].dir_pin, GPIO_PIN_RESET);
    }
}

HAL_StatusTypeDef MotorHw_StartRelative(const int32_t signed_pulses[4], uint32_t base_rate_hz)
{
    uint32_t max_pulses = 0U;
    uint32_t pulses[4] = {0U};
    uint32_t rate = 0U;
    uint8_t i = 0;

    if (MotorHw_AnyBusy() != 0U)
    {
        return HAL_BUSY;
    }

    for (i = 0; i < 4U; ++i)
    {
        pulses[i] = (uint32_t)labs(signed_pulses[i]);
        if (pulses[i] > max_pulses)
        {
            max_pulses = pulses[i];
        }
    }

    if (max_pulses == 0U)
    {
        return HAL_OK;
    }

    for (i = 0; i < 4U; ++i)
    {
        if (pulses[i] == 0U)
        {
            continue;
        }

        rate = (base_rate_hz * pulses[i]) / max_pulses;
        if (rate < MIN_MOTOR_PULSE_RATE_HZ)
        {
            rate = MIN_MOTOR_PULSE_RATE_HZ;
        }

        MotorHw_StartSingle(
            (MotorId)i,
            (signed_pulses[i] >= 0) ? MOTOR_DIR_IN : MOTOR_DIR_OUT,
            pulses[i],
            rate);
    }

    return HAL_OK;
}

void MotorHw_Stop(MotorId id)
{
    MotorChannel *motor;

    if (id > MOTOR_4)
    {
        return;
    }

    motor = &s_motors[id];
    HAL_TIM_Base_Stop_IT(motor->htim);
    HAL_GPIO_WritePin(motor->step_port, motor->step_pin, GPIO_PIN_RESET);
    motor->step_level = 0U;
    motor->running = 0U;
}

void MotorHw_StopAll(void)
{
    uint8_t i = 0;
    for (i = 0; i < 4U; ++i)
    {
        MotorHw_Stop((MotorId)i);
    }
}

uint8_t MotorHw_IsBusy(MotorId id)
{
    if (id > MOTOR_4)
    {
        return 0U;
    }
    return s_motors[id].running;
}

uint8_t MotorHw_AnyBusy(void)
{
    uint8_t i = 0;
    for (i = 0; i < 4U; ++i)
    {
        if (s_motors[i].running != 0U)
        {
            return 1U;
        }
    }
    return 0U;
}

void MotorHw_TimerElapsedCallback(TIM_HandleTypeDef *htim)
{
    uint8_t i = 0;
    MotorChannel *motor = NULL;

    for (i = 0; i < 4U; ++i)
    {
        if (s_motors[i].htim == htim)
        {
            motor = &s_motors[i];
            break;
        }
    }

    if ((motor == NULL) || (motor->running == 0U))
    {
        return;
    }

    motor->step_level ^= 1U;
    HAL_GPIO_WritePin(motor->step_port, motor->step_pin, motor->step_level ? GPIO_PIN_SET : GPIO_PIN_RESET);

    if (motor->step_level != 0U)
    {
        motor->current_pulses++;
        if (motor->current_pulses >= motor->target_pulses)
        {
            HAL_GPIO_WritePin(motor->step_port, motor->step_pin, GPIO_PIN_RESET);
            motor->step_level = 0U;
            motor->running = 0U;
            HAL_TIM_Base_Stop_IT(motor->htim);
        }
    }
}
