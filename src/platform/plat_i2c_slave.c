/*
  NAME: I2C SLAVE INIT
  FILE: plat_i2c_slave.c
  DESCRIPTION: Provide i2c slave config table "I2C_SLAVE_CFG_TABLE[]" for init slave config.
  AUTHOR: MouchenHung
  DATE/VERSION: 2021.11.26 - v1.1
  Note: 
    (1) "plat_i2c_slave.h" is included by "hal_i2c_slave.h"
*/

#include <zephyr.h>
#include <stdio.h>
#include <stdlib.h>
#include "plat_i2c_slave.h"

/* I2C slave init-config table */
const struct _i2c_slave_config I2C_SLAVE_CFG_TABLE[MAX_SLAVE_NUM] = {
  {"I2C_0",   0x60,   0xA,    SLAVE_ENABLE },
  {"I2C_1",   0x40,   0xA,    SLAVE_ENABLE  },
  {"I2C_2",   0xFF,   0xA,    SLAVE_DISABLE },
  {"I2C_3",   0xFF,   0xA,    SLAVE_DISABLE },
  {"I2C_4",   0xFF,   0xA,    SLAVE_DISABLE },
  {"I2C_5",   0xFF,   0xA,    SLAVE_DISABLE },
  {"I2C_6",   0xFF,   0xA,    SLAVE_DISABLE },
  {"I2C_7",   0xFF,   0xA,    SLAVE_DISABLE },
  {"I2C_8",   0xFF,   0xA,    SLAVE_DISABLE },
  {"I2C_9",   0xFF,   0xA,    SLAVE_DISABLE },
  {"I2C_10",  0xFF,   0xA,    SLAVE_DISABLE },
  {"I2C_11",  0xFF,   0xA,    SLAVE_DISABLE },
  {"I2C_12",  0xFF,   0xA,    SLAVE_DISABLE },
  {"I2C_13",  0xFF,   0xA,    SLAVE_DISABLE },
  {"I2C_14",  0xFF,   0xA,    SLAVE_DISABLE },
  {"I2C_15",  0xFF,   0xA,    SLAVE_DISABLE }
};