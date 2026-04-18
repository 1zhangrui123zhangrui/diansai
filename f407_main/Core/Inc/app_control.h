#ifndef APP_CONTROL_H
#define APP_CONTROL_H

#include <stdint.h>
#include "types.h"

void AppControl_Init(void);
void AppControl_Task(uint32_t now_ms);
void AppControl_RequestReset(void);
void AppControl_RequestEdgePatrol(void);
void AppControl_LoadManualTargets(const Point2D *points, uint8_t count);
AppState AppControl_GetState(void);
uint8_t AppControl_GetFireCount(void);
uint8_t AppControl_GetFirePoint(uint8_t index, Point2D *point);

#endif
