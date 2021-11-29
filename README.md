# PROJ_Q_YV3p5_I2C_SLAVE
I2C slave device relative code.
======================================================================================================================================
OVERLOOK - LATEST
======================================================================================================================================
PHASE1. DEVICE CODE	v1.2	2021.11.25
PHASE2. PLATFORM CODE	v1.0	2021.11.25

======================================================================================================================================
COMMIT CONTEXT
======================================================================================================================================
{ PHASE1. DEVICE CODE }
  NAME: I2C SLAVE DEVICE
  FILE: hal_i2c_slave.c
  DESCRIPTION: There is 1 callback function "i2c_slave_cb" for I2C slave ISR handle and user APIs "q_*" for user access.
  AUTHOR: MouchenHung
  DATE/VERSION: 2021.11.25 - v1.2
  Note: 
    (1) Shall not modify code in this file!!!

    (2) "hal_i2c_slave.h" must be included!

    (3) "util_init_I2C_slave()" is required before any function or api, cause mutex lock create inside it.

    (4) User APIs follow check-rule before doing task 
          [api]                               [.is_enable] [.is_init] [.is_register]
        * q_i2c_slave_enable                  X            X          X
        * q_i2c_slave_init                    O            X          X
        * q_i2c_slave_register                O            O          X
        * q_i2c_slave_unregister              O            O          X
        * q_i2c_slave_read                    O            O          X
        * q_i2c_slave_status_get              X            X          X
        * q_i2c_slave_status_print            X            O          X
                                              (O: must equal 1, X: no need to check)

    (5) I2C slave function/api usage recommend
        [RUN TIME ACTION]
        * If doesn't set before, and want to let one bus slave function work 
          "q_i2c_slave_enable()" --> "q_i2c_slave_init()" --> "q_i2c_slave_register()"
        * If want to disable one running bus and prevent relative user api usage by user
          "q_i2c_slave_unregister()" --> "q_i2c_slave_enable()"
        * If want to modify some config element, if already register before
          "q_i2c_slave_unregister()" --> "q_i2c_slave_init()" --> "q_i2c_slave_register()"
        [READ]
        Able to use only if i2c slace is enable and init

    (6) Slave queue method: Zephyr api, unregister the bus while full msgq, register back while msgq get space.
*/
_____HISTORY___________________________________________________________________________________________________________________
v1.0 - 2021.11.23 - First commit
v1.1 - 2021.11.24 - Code modify1
		    * Simplify code
		    * Remove".is_msg_full" from struct "i2c_slave_device"
		    * Modify "MAX_I2C_SLAVE_BUFF" from 255 to 512
		    * Remove first byte (slave address) from buffer in slave callback function
		    * Add new input arg from "util_init_I2C_slave()", to control whether need to do slave register after init
		    * Add "util_register_I2C_slave()" to user api
v1.2 - 2021.11.25 - Code modify2
		    * Solve message queue memory free issue
		    * Add platform code to exclude modifiable code from previous "hal_i2c_slave.c"
		    * Remove init and register action in function "util_init_I2C_slave()", which means additional action is needed in main function
		    * Move i2c slave table "I2C_SLAVE_CFG_TABLE[]" and "util_init_I2C_slave()" to "plat_i2c_slave.c" as platform file

{ PHASE2. PLATFORM CODE }
  NAME: I2C SLAVE INIT
  FILE: plat_i2c_slave.c
  DESCRIPTION: Provide i2c slave config table "I2C_SLAVE_CFG_TABLE[]" for init and "util_init_I2C_slave()" for user access.
  AUTHOR: MouchenHung
  DATE/VERSION: 2021.11.25 - v1.0
  Note: 
    (1) "plat_i2c_slave.h" is included by "hal_i2c_slave.h"

    (2) "util_init_I2C_slave()" is required before any function or api, cause mutex lock create inside it.

    (3) I2C slave function/api usage recommend
        [INIT]
        * Common init
          "util_init_I2C_slave()" --> "q_i2c_slave_register()"

_____HISTORY___________________________________________________________________________________________________________________
v1.0 - 2021.11.25 - First commit



======================================================================================================================================
USAGE
======================================================================================================================================
[STEP1. Set bus and address main.c]
	static mctp_smbus_port smbus_port[MCTP_SMBUS_NUM] = {
		{.conf.smbus_conf.addr = 0x60, .conf.smbus_conf.bus = 0x00},
		{.conf.smbus_conf.addr = 0x40, .conf.smbus_conf.bus = 0x01}
	};

[STEP2. Add i2c slave activate function to main.c]
	void do_init_I2C_slave(void) {
		uint8_t ret = 0;

		/* Init i2c slave global and mutex */
		util_init_I2C_slave();

	  	/* Parsing slave config only if bus is enable */
	  	for (int i=0; i<MAX_SLAVE_NUM; i++){
	    	if (I2C_SLAVE_CFG_TABLE[i].enable){
				ret = q_i2c_slave_init(i, I2C_SLAVE_CFG_TABLE[i].controller_dev_name, I2C_SLAVE_CFG_TABLE[i].address, I2C_SLAVE_CFG_TABLE[i].i2c_msg_count);
				if (ret)
					printk("do_init_I2C_slave: Init bus[%d] slave - failed, cause of errorcode[%d]\n", i, ret);
				else
					printk("+ Init bus[%d] slave - success!\n", i);

				ret = q_i2c_slave_register(i);
				if (ret)
					printk("do_init_I2C_slave: Register bus[%d] slave - failed, cause of errorcode[%d]\n", i, ret);
	      		else
				printk("+ Register bus[%d] slave - success!\n", i);
			}
	  	}
	}

[STEP3. Add init function to main.c "main()"]
	....
	util_init_I2C();

	do_init_I2C_slave();

	uint32_t i;
	for (i = 0; i < MCTP_SMBUS_NUM; i++) {
	....

[STEP3. Add read api to "mctp_smbus_read"]
	...
	uint8_t ret = q_i2c_slave_read(mctp_inst->medium_conf.smbus_conf.bus, rdata, ARRAY_SIZE(rdata), &rlen);
	if (ret){
	printk("Error occur: couse of errorcode[%d]\n", ret);
	}

	//rlen--;
	//memmove(&rdata[0], &rdata[1], rlen);
	...
