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

/* I2C slave init-enable table */
static const bool I2C_SLAVE_EN_TABLE[MAX_SLAVE_NUM] = {
  SLAVE_ENABLE,
  SLAVE_ENABLE,
  SLAVE_DISABLE,
  SLAVE_DISABLE,
  SLAVE_DISABLE,
  SLAVE_DISABLE,
  SLAVE_DISABLE,
  SLAVE_DISABLE,
  SLAVE_DISABLE,
  SLAVE_DISABLE,
  SLAVE_DISABLE,
  SLAVE_DISABLE,
  SLAVE_DISABLE,
  SLAVE_DISABLE,
  SLAVE_DISABLE,
  SLAVE_DISABLE
};

/* I2C slave init-config table */
static const struct _i2c_slave_config I2C_SLAVE_CFG_TABLE[MAX_SLAVE_NUM] = {
  { 0x60,   0x2 },
  { 0x40,   0x2 },
  { 0xFF,   0x2 },
  { 0xFF,   0xA },
  { 0xFF,   0xA },
  { 0xFF,   0xA },
  { 0xFF,   0xA },
  { 0xFF,   0xA },
  { 0xFF,   0xA },
  { 0xFF,   0xA },
  { 0xFF,   0xA },
  { 0xFF,   0xA },
  { 0xFF,   0xA },
  { 0xFF,   0xA },
  { 0xFF,   0xA },
  { 0xFF,   0xA }
};

void util_init_I2C_slave(void) {
	int ret = 0;
	LOG_INF("<system> Start init i2c slave bus with init config-table!\n");

	struct _i2c_slave_config *cur_cfg = (struct _i2c_slave_config *)malloc(sizeof(struct _i2c_slave_config));

  for (int i=0; i<MAX_SLAVE_NUM; i++){
    if (I2C_SLAVE_EN_TABLE[i]){
      cur_cfg->address = I2C_SLAVE_CFG_TABLE[i].address;
      cur_cfg->i2c_msg_count = I2C_SLAVE_CFG_TABLE[i].i2c_msg_count;

      ret = i2c_slave_control(i, cur_cfg, I2C_CONTROL_REGISTER);
      if (ret)
        LOG_ERR("Init bus[%d] slave - failed, cause of errorcode[%d]\n", i, ret);
      else
        LOG_INF("+ Activate bus[%d] slave - success!\n", i);
    }
  }

	free(cur_cfg);
}