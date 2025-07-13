//
// Created by maksim on 12.07.25.
//

#include "bsp.h"
#include "fmc.h"
#include "i2c.h"

static void BSP_SDRAM_Init(void) {
    FMC_SDRAM_CommandTypeDef Command;

    __IO uint32_t tmpmrd = 0;

    Command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber = 1;
    Command.ModeRegisterDefinition = 0;
    HAL_SDRAM_SendCommand(&hsdram1, &Command, (uint32_t) 0x1000);

    HAL_Delay(1);

    Command.CommandMode = FMC_SDRAM_CMD_PALL;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber = 1;
    Command.ModeRegisterDefinition = 0;
    HAL_SDRAM_SendCommand(&hsdram1, &Command, (uint32_t) 0x1000);

    Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber = 8;
    Command.ModeRegisterDefinition = 0;
    HAL_SDRAM_SendCommand(&hsdram1, &Command, (uint32_t) 0x1000);

    tmpmrd = (uint32_t) 0x0001 | 0x0000 | 0x0030 | 0x0000 | 0x0200;

    Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber = 1;
    Command.ModeRegisterDefinition = tmpmrd;
    HAL_SDRAM_SendCommand(&hsdram1, &Command, (uint32_t) 0x1000);

    HAL_SDRAM_ProgramRefreshRate(&hsdram1, 918);
}

static void GT911_WR_Reg_One_byte(uint16_t reg, uint8_t data) {
    uint8_t temp[3] = {0};
    temp[0] = reg >> 8;
    temp[1] = reg & 0xff;
    temp[2] = data;
    HAL_I2C_Master_Transmit(&hi2c4, 0XBA, temp, 3, 0xff);
}

static void GT911_RD_Reg(uint16_t reg, uint8_t *buf, uint8_t len) {
    uint8_t temp[2] = {0};
    temp[1] = reg & 0xff;
    temp[0] = reg >> 8;
    HAL_I2C_Master_Transmit(&hi2c4, 0XBA, temp, 2, 0xff);
    HAL_I2C_Master_Receive(&hi2c4, 0XBB, buf, len, 0xff);
}

static void BSP_TS_Init(void) {
    HAL_Delay(100);
    uint8_t temp[5] = {0};
    HAL_GPIO_WritePin(TS_RST_GPIO_Port, TS_RST_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TS_INT_GPIO_Port, TS_INT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TS_RST_GPIO_Port, TS_RST_Pin, GPIO_PIN_SET);

    temp[4] = 0;
    GT911_RD_Reg(0X8140, temp, 4);

    temp[0] = 0X02;
    GT911_WR_Reg_One_byte(0X8040, temp[0]);
    GT911_RD_Reg(0X8047, temp, 1);
}

void BSP_TS_GetState(TS_StateTypeDef *state) {
    static uint8_t counts = 0;
    uint8_t flag = 0;
    uint8_t buf[4] = {0};

    GT911_RD_Reg(0X814E, &flag, 1);
    if (flag & 0x80) {
        GT911_WR_Reg_One_byte(0X814E, 0);
        if (flag & 0xf) {
            GT911_RD_Reg(0X8150, buf, 4);
            state->X = (uint16_t) ((buf[1] << 8) | buf[0]);
            state->Y = (uint16_t) ((buf[3] << 8) | buf[2]);
            state->TouchDetected = 1;
            GT911_WR_Reg_One_byte(0X814E, 0);
        } else
            state->TouchDetected = 0;
    } else
        state->TouchDetected = 0;

    counts++;
}

void BSP_Init(void) {
    BSP_SDRAM_Init();
    BSP_TS_Init();
}
