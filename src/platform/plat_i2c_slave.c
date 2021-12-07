/*
  NAME: I2C SLAVE INIT
  FILE: plat_i2c_slave.c
  DESCRIPTION: Provide "util_init_I2C_slave()" for init slave config.
  AUTHOR: MouchenHung
  DATE/VERSION: 2021.12.06 - v1.2
  Note: 
    (1) "plat_i2c_slave.h" is included by "hal_i2c_slave.h"
*/

#include <zephyr.h>
#include <stdio.h>
#include <stdlib.h>
#include "plat_i2c_slave.h"

/* LOG SET */
#include <logging/log.h>
LOG_MODULE_DECLARE(mc_i2c_slave);

/* I2C slave init-config table, disable if slave address equal 0xFF */
static const struct _i2c_slave_config I2C_SLAVE_CFG_TABLE[MAX_SLAVE_NUM] = {
  { 0x60,       0x05 },
  { 0x40,       0x05 },
  { MAX_ADDR,   0x0A },
  { MAX_ADDR,   0x0A },
  { MAX_ADDR,   0x0A },
  { MAX_ADDR,   0x0A },
  { MAX_ADDR,   0x0A },
  { MAX_ADDR,   0x0A },
  { MAX_ADDR,   0x0A },
  { MAX_ADDR,   0x0A },
  { MAX_ADDR,   0x0A },
  { MAX_ADDR,   0x0A },
  { MAX_ADDR,   0x0A },
  { MAX_ADDR,   0x0A },
  { MAX_ADDR,   0x0A },
  { MAX_ADDR,   0x0A }
};

void util_init_I2C_slave(void) {
	int ret = 0;
	LOG_INF("<system> Start init i2c slave bus with init config-table!\n");

  for (int i=0; i<MAX_SLAVE_NUM; i++){
    if (I2C_SLAVE_CFG_TABLE[i].address != MAX_ADDR){
      ret = i2c_slave_control(i, &I2C_SLAVE_CFG_TABLE[i], I2C_CONTROL_REGISTER);
      if (ret)
        LOG_ERR("Init bus[%d] slave - failed, cause of errorcode[%d]\n", i, ret);
      else
        LOG_INF("+ Activate bus[%d] slave - success!\n", i);
    }
  }
}