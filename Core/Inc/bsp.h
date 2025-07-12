//
// Created by maksim on 12.07.25.
//

#ifndef BSP_H
#define BSP_H

#include "main.h"

typedef struct {
    uint16_t TouchDetected;
    uint16_t X;
    uint16_t Y;
} TS_StateTypeDef;

void BSP_Init(void);

void BSP_TS_GetState(TS_StateTypeDef *state);

#endif //BSP_H
