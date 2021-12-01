# PROJ_Q_YV3p5_I2C_SLAVE
I2C slave device relative code.
##### ==================================================================================
### OVERLOOK - LATEST
##### ==================================================================================
###### **PHASE1**. DEVICE CODE		v1.2	2021.11.25
###### **PHASE2**. PLATFORM CODE	v1.0	2021.11.25

##### ==================================================================================
### COMMIT CONTEXT
##### ==================================================================================
### { PHASE1. DEVICE CODE }
**NAME**: I2C SLAVE DEVICE<br>
**FILE**: hal_i2c_slave.c<br>
**DESCRIPTION**: There is 1 callback function "i2c_slave_cb" for I2C slave ISR handle and user APIs "q_*" for user access.<br>
**AUTHOR**: MouchenHung<br>
**DATE/VERSION**: 2021.12.01 - v1.4<br>
**Note**: <br>
    (1) Shall not modify code in this file!!!<br>
<br>
    (2) "hal_i2c_slave.h" must be included!<br>
<br>
    (3) User APIs follow check-rule before doing task <br>
```
          [api]                               [.is_init] [.is_register]
        * i2c_slave_control                   X          X
        * i2c_slave_read                      O          X
        * i2c_slave_status_get                X          X
        * i2c_slave_status_print              X          X
        * i2c_slave_cfg_get                   O          X
                                              (O: must equal 1, X: no need to check)
```
<br>
    (4) I2C slave function/api usage recommend<br>
        [ACTIVATE]<br>
          Use "i2c_slave_control()" to register/modify/unregister slave bus<br>
        [READ]<br>
          Use "i2c_slave_read()" to read slave queue message<br>
<br>
    (5) Slave queue method: Zephyr api, unregister the bus while full msgq, register back while msgq get space.<br>
<br>

##### _____HISTORY______________________________________________________________________
**v1.0 - 2021.11.23** - First commit<br>
**v1.1 - 2021.11.24** - Code modify1<br>
		    * Simplify code<br>
		    * Remove".is_msg_full" from struct "i2c_slave_device"<br>
		    * Modify "MAX_I2C_SLAVE_BUFF" from 255 to 512<br>
		    * Remove first byte (slave address) from buffer in slave callback function<br>
		    * Add new input arg from "util_init_I2C_slave()", to control whether need to do slave register after init<br>
		    * Add "util_register_I2C_slave()" to user api<br>
**v1.2 - 2021.11.25** - Code modify2<br>
		    * Solve message queue memory free issue<br>
		    * Add platform code to exclude modifiable code from previous "hal_i2c_slave.c"<br>
		    * Remove init and register action in function "util_init_I2C_slave()", which means additional action is needed in main function<br>
		    * Move i2c slave table "I2C_SLAVE_CFG_TABLE[]" and "util_init_I2C_slave()" to "plat_i2c_slave.c" as platform file<br>
**v1.2 - 2021.12.01** - Code modify3<br>
		    * Bug fixed<br>
		    * Simplify code<br>
<br>
### { PHASE2. PLATFORM CODE }
**NAME**: I2C SLAVE INIT<br>
**FILE**: plat_i2c_slave.c<br>
**DESCRIPTION**: Provide i2c slave config table "I2C_SLAVE_CFG_TABLE[]" for init slave config.<br>
**AUTHOR**: MouchenHung<br>
**DATE/VERSION**: 2021.11.25 - v1.0<br>
**Note**: <br>
    (1) "plat_i2c_slave.h" is included by "hal_i2c_slave.h"<br>

##### _____HISTORY______________________________________________________________________
**v1.0** - 2021.11.25 - First commit<br>
<br>
##### ==================================================================================
### USAGE
##### ==================================================================================
##### [STEP1. Set bus and address main.c]
```c
	static mctp_smbus_port smbus_port[MCTP_SMBUS_NUM] = {
		{.conf.smbus_conf.addr = 0x60, .conf.smbus_conf.bus = 0x00},
		{.conf.smbus_conf.addr = 0x40, .conf.smbus_conf.bus = 0x01}
	};
```
##### [STEP2. Add i2c slave activate function to main.c]
```c
	void do_init_I2C_slave(void) {
		uint8_t ret = 0;

	  	/* Parsing slave config only if bus is enable */
	  	for (int i=0; i<MAX_SLAVE_NUM; i++){
			if (I2C_SLAVE_CFG_TABLE[i].enable){
				struct _i2c_slave_config *cur_cfg = (struct _i2c_slave_config *)malloc(sizeof(struct _i2c_slave_config));
				cur_cfg->controller_dev_name = NULL; //no affect
				cur_cfg->enable = 0; //no affect
				cur_cfg->address = I2C_SLAVE_CFG_TABLE[i].address;
				cur_cfg->i2c_msg_count = I2C_SLAVE_CFG_TABLE[i].i2c_msg_count;

				ret = i2c_slave_control(i, cur_cfg, I2C_CONTROL_REGISTER);
				if (ret)
					printk("do_init_I2C_slave: Init bus[%d] slave - failed, cause of errorcode[%d]\n", i, ret);
				else
					printk("+ Init bus[%d] slave -   success!\n", i);
			}
	  	}
	}
```
##### [STEP3. Add init function to main.c "main()"]
```c
	do_init_I2C_slave();

	uint32_t i;
	for (i = 0; i < MCTP_SMBUS_NUM; i++) {
```

##### [STEP3. Add read api to "mctp_smbus_read"]
```c
	uint8_t ret = i2c_slave_read(mctp_inst->medium_conf.smbus_conf.bus, rdata, ARRAY_SIZE(rdata), &rlen);
	if (ret){
	printk("Error occur: couse of errorcode[%d]\n", ret);
	}
```
