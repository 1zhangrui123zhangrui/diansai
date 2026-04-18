#include "fire_link.h"

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
    GPIO_PinState active_state;
    uint32_t debounce_ms;
    uint32_t last_change_tick;
    uint8_t active;
    uint8_t new_event;
    uint8_t initialized;
} FireLinkContext;

static FireLinkContext s_fire_link;

static uint8_t FireLink_ReadPinActive(void)
{
    GPIO_PinState state;

    if (s_fire_link.initialized == 0U)
    {
        return 0U;
    }

    state = HAL_GPIO_ReadPin(s_fire_link.port, s_fire_link.pin);
    return (state == s_fire_link.active_state) ? 1U : 0U;
}

void FireLink_Init(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState active_state, uint32_t debounce_ms)
{
    s_fire_link.port = port;
    s_fire_link.pin = pin;
    s_fire_link.active_state = active_state;
    s_fire_link.debounce_ms = debounce_ms;
    s_fire_link.last_change_tick = HAL_GetTick();
    s_fire_link.new_event = 0U;
    s_fire_link.initialized = 1U;
    s_fire_link.active = FireLink_ReadPinActive();
}

void FireLink_Task(uint32_t now_ms)
{
    uint8_t pin_active;

    if (s_fire_link.initialized == 0U)
    {
        return;
    }

    pin_active = FireLink_ReadPinActive();
    if (pin_active != s_fire_link.active)
    {
        if ((now_ms - s_fire_link.last_change_tick) >= s_fire_link.debounce_ms)
        {
            s_fire_link.active = pin_active;
            s_fire_link.last_change_tick = now_ms;
            if (pin_active != 0U)
            {
                s_fire_link.new_event = 1U;
            }
        }
    }
    else
    {
        s_fire_link.last_change_tick = now_ms;
    }
}

uint8_t FireLink_IsActive(void)
{
    return s_fire_link.active;
}

uint8_t FireLink_HasNewEvent(void)
{
    return s_fire_link.new_event;
}

void FireLink_ClearEvent(void)
{
    s_fire_link.new_event = 0U;
}
