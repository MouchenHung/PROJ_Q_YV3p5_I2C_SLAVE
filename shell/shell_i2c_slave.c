/*
 * NONE
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <logging/log.h>
#include <shell/shell.h>

#include "hal_i2c.h"
#include "hal_i2c_slave.h"
#include "i2c_slave_case.h"

#include <device.h>
#include <devicetree.h>

#define SHELL_VERSION "v1.0"
#define SHELL_DATE "2021.11.15"

LOG_MODULE_REGISTER(i2c_slave_shell, CONFIG_I2C_LOG_LEVEL);

#define DT_DRV_COMPAT aspeed_i2c

#define I2C_DEVICE_PREFIX "I2C_"

static int i2c_controller_check(uint8_t bus_num){
	char ControlerName[10] = "I2C_";
	char num[10];

	sprintf(num, "%d", (int)bus_num);
	strcat(ControlerName, num);

	const struct device *tmp_device = device_get_binding(ControlerName);

    if (!tmp_device) {
	  return 1;
    }
	return 0;
}

static int cmd_info_print(const struct shell *shell, size_t argc, char **argv){
	shell_print(shell, "========================{SHELL COMMAND INFO}========================");
	shell_print(shell, "* DESCRIPTION:   I2C slave test command");
	shell_print(shell, "* AUTHOR:        MouchenHung");
	shell_print(shell, "* DATE/VERSION:  %s - %s", SHELL_DATE, SHELL_VERSION);
	shell_print(shell, "* Note:          You know, I'm Mouchen Hung, the most handsome one!!");
	shell_print(shell, "========================{SHELL COMMAND INFO}========================");

	return 0;
}

static void cmd_i2c_slave_get_status(const struct shell *shell, size_t argc, char **argv){
	if (argc != 2){
		shell_print(shell, "Try: i2c_test slave_init <slave_bus>");
		return;
	}

	uint8_t slave_bus = strtol(argv[1], NULL, 10);

	if (slave_bus > MAX_SLAVE_NUM-1){
		shell_print(shell, "<warning> Bus number over limit %d(zero base)!", MAX_SLAVE_NUM-1);
		return;
	}

	if (i2c_controller_check(slave_bus)){
		shell_print(shell, "<warning> Bus %d controller is not enable in device tree!", slave_bus);
		return;
	}

	if (q_i2c_slave_status_print(slave_bus)){
		shell_print(shell, "<error> Bus %d can't reach current status!", slave_bus);
		return;
	}
}

static void cmd_i2c_slave_register(const struct shell *shell, size_t argc, char **argv){
	if (argc != 5){
		shell_print(shell, "Try: i2c_test slave_register <slave_bus> <slave_addr> <max_msg_num> <1:register/0:unregister>");
		return;
	}

	uint8_t slave_bus = strtol(argv[1], NULL, 10);
	uint8_t slave_addr = strtol(argv[2], NULL, 16);
	uint16_t max_msg_num = strtol(argv[3], NULL, 10);
	uint8_t register_flag = strtol(argv[4], NULL, 10);
	int ret = 0;

	if (slave_bus > MAX_SLAVE_NUM-1){
		shell_print(shell, "<warning> Bus number over limit %d(zero base)!", MAX_SLAVE_NUM-1);
		return;
	}

	if (i2c_controller_check(slave_bus)){
		shell_print(shell, "<warning> Bus %d controller is not enable in device tree!", slave_bus);
		return;
	}

	char ControlerName[10] = "I2C_";
	char num[10];
	sprintf(num, "%d", slave_bus);
	strcat(ControlerName, num);

	struct _i2c_slave_config *cur_cfg;

	if (register_flag != 0 && register_flag != 1){
		shell_print(shell, "<warning> register flag only accept 1 or 0");
	}
	else if (register_flag){
		cur_cfg = (struct _i2c_slave_config *)malloc(sizeof(struct _i2c_slave_config));
		cur_cfg->address = slave_addr;
		cur_cfg->controller_dev_name = ControlerName;
		cur_cfg->i2c_msg_count = max_msg_num;
		cur_cfg->enable = 0; //no affect

		ret = q_i2c_slave_control(slave_bus, cur_cfg, I2C_CONTROL_REGISTER);
		if (ret){
			shell_print(shell, "<error> I2c slave register bus failed with errorcode %d!", ret);
		}
		else{
			shell_print(shell, "--> I2c slave register bus success!");
		}
	}
	else{
		cur_cfg = NULL;
		ret = q_i2c_slave_control(slave_bus, cur_cfg, I2C_CONTROL_UNREGISTER);
		if (ret){
			shell_print(shell, "<error> I2c slave unregister bus failed with errorcode %d!", ret);
		}
		else{
			shell_print(shell, "--> I2c slave unregister bus success!");
		}
	}
}

static void cmd_i2c_slave_read(const struct shell *shell, size_t argc, char **argv){
	if (argc != 2){
		shell_print(shell, "Try: i2c_test slave_readsingle <slave_bus>");
		return;
	}

	uint8_t slave_bus = strtol(argv[1], NULL, 10);

	if (slave_bus > MAX_SLAVE_NUM-1){
		shell_print(shell, "<warning> Bus number over limit %d(zero base)!", MAX_SLAVE_NUM-1);
		return;
	}

	if (i2c_controller_check(slave_bus)){
		shell_print(shell, "<warning> Bus %d controller is not enable in device tree!", slave_bus);
		return;
	}

	shell_print(shell, "Read I2C slave queue on bus[%d]", slave_bus);

	uint16_t max_slave_read = 30;
	uint8_t *msg = (uint8_t*)malloc(max_slave_read * sizeof(uint8_t));
	uint8_t ipmb_buffer_rx[30];
	uint16_t rx_len = 0;
	uint8_t ret = 0;
	
	ret = q_i2c_slave_read(slave_bus, msg, max_slave_read, &rx_len);
	if (!ret){
		if (rx_len){
			memcpy(ipmb_buffer_rx, (uint8_t *)msg, rx_len);
			//ipmb_buffer_rx[0] = ipmb_buffer_rx[0] >> 1;

			shell_print(shell, "--> Slave read: ");
			shell_hexdump(shell, ipmb_buffer_rx, rx_len);
		}
		else{
			shell_print(shell, "<warning> Slave msg queue is currently empty!");
		}
	} 
	else{
		shell_print(shell, "<error> Slave read failed, cause of errorcode %d!", ret);
	}
}

static void cmd_i2c_slave_write(const struct shell *shell, size_t argc, char **argv){
	if (argc < 5){
		shell_print(shell, "Try: i2c_test master_writesingle <master_bus> <slave_addr> <msg_len> <data...>");
		return;
	}
	uint8_t master_bus = strtol(argv[1], NULL, 16);
	uint8_t slave_addr = strtol(argv[2], NULL, 16);
	uint32_t wr_length = strtol(argv[3], NULL, 16);
	uint32_t rd_length = 2;
	I2C_MSG i2c_msg;
	uint8_t retry = 3;
	uint8_t ret = 0;

	if (!wr_length){
		shell_print(shell, "<warning> Write with empty message!");
		return;
	}
	else if (wr_length > 30){
		shell_print(shell, "<warning> Write length is over 30 bytes!");
		return;
	}
	else {
		if (master_bus > MAX_SLAVE_NUM-1){
			shell_print(shell, "<warning> Bus number over limit %d(zero base)!", MAX_SLAVE_NUM-1);
			return;
		}

		if (i2c_controller_check(master_bus)){
			shell_print(shell, "<warning> Bus %d controller is not enable in device tree!", master_bus);
			return;
		}

		if ((argc-4) != wr_length){
			shell_print(shell, "<warning> Write bytes number is not match with <msg_len>!");
			return;
		}
		shell_print(shell, "Write I2C slave queue on bus[%d] to slave[%d] with msg length[%d]", master_bus, slave_addr, wr_length);
		i2c_msg.bus = master_bus;
		i2c_msg.slave_addr = slave_addr;
		i2c_msg.rx_len = rd_length;
		i2c_msg.tx_len = wr_length;

		for (int i = 0; i < wr_length; i++) {
			i2c_msg.data[i] = (uint8_t)strtol(argv[4 + i], NULL, 16);
		}

		ret = i2c_master_write(&i2c_msg, retry);
		if (ret){
			shell_print(shell, "<error> write error: %d", ret);
		}
		else{
			shell_print(shell, "--> write success!");
		}
	}
}

static void cmd_i2c_slave_test(const struct shell *shell, size_t argc, char **argv){
#if EVB_BOARD
	if (argc != 2){
		shell_print(shell, "Try: i2c_test master_writesingle <case_num>");
		return;
	}

	uint8_t case_num = strtol(argv[1], NULL, 10);

	switch (case_num)
	{
	case 1:
		test_case1();
		break;

	case 2:
		test_case2_1();
		break;
	
	case 3:
		test_case2_2();
		break;

	case 4:
		test_case3();
		break;

	case 5:
		test_case4();
		break;
	
	case 6:
#if MCTP_CMD_SUPPORT == 1
		test_case5();
#endif
		break;
	
	default:
		shell_print(shell, "<warning> Test case not exist!");
		shell_print(shell, "Try case_num [1]: test case 1    Testing only 1 loop");
		shell_print(shell, "Try case_num [2]: test case 2-1  Change config then test 1 loop");
		shell_print(shell, "Try case_num [3]: test case 2-2  Dynamic modify then test 1 loop");
		shell_print(shell, "Try case_num [4]: test case 3    Test for slave queue with loops");
		shell_print(shell, "Try case_num [5]: test case 4    Stress for msgq memory while modify slave config");
		shell_print(shell, "Try case_num [6]: test case 5    MCTP command test");
		break;
	}
#else
	shell_print(shell, "Command only support for Ast1030 evb board!!!");
#endif
}

#if MCTP_CMD_SUPPORT == 1
static void tmp_master_cb(void *args, uint8_t *buf, uint16_t len){
	printk("\n");
	printk("!!!MASTER RECEIVE SLAVE FEEDBACK MESSAGE!!!\n--> ");
	for (int i=0; i<len; i++){
		printk("0x%x ", *(buf+i));
	}
	printk("\n");
}
#endif

static void cmd_i2c_mctp(const struct shell *shell, size_t argc, char **argv){
	if (argc != 3){
		shell_print(shell, "Try: i2c_test master_write_mctp_cmd <loop_num> <port>");
		return;
	}

#if MCTP_CMD_SUPPORT == 1
	uint8_t loop_num = strtol(argv[1], NULL, 10);
	uint8_t port = strtol(argv[2], NULL, 10);

	uint8_t loop = 0;
	while (loop < loop_num)
	{
		printk("[loop] mctp send command\n");
		k_msleep(1000);

		/* TODO: Sendout mctp command */
		mctp_ext_param ext_param = {0};
		ext_param.type = MCTP_MEDIUM_TYPE_SMBUS;
		ext_param.smbus_ext_param.addr = 0x40;
		ext_param.ep = 0x0a;

		pldm_msg msg = {0};
		msg.hdr.pldm_type = PLDM_TYPE_BASE;
		msg.hdr.cmd = PLDM_BASE_CMD_CODE_GETTID;
		msg.hdr.rq = 1;

		mctp_pldm_send_msg(smbus_port[port].mctp_inst, &msg, ext_param, tmp_master_cb, NULL);
		
		loop++;
	}
#else
	shell_print(shell, "MCTP command not support!");
#endif
	
}

enum task0_mode {
	/** The sequence should be continued normally. */
	BUS0,
	BUS1,
	BUS2,
	BUS3,
	BUS4,
	TASK0_MODE_INVALID = 0xFF,
};

static void i2c_controler_get(size_t idx, struct shell_static_entry *entry)
{
	char NUM[2];
	const struct device *dev = shell_device_lookup(idx, I2C_DEVICE_PREFIX);
	if (dev){
		sprintf(NUM, "%d", idx);
		entry->syntax = NUM;
		entry->handler = NULL;
		entry->help = NULL;
		entry->subcmd = NULL;
		return;
	}
	entry->syntax = NULL;
}

SHELL_DYNAMIC_CMD_CREATE(bus_number_find, i2c_controler_get);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_i2c_slave_cmds,
					SHELL_CMD(note, NULL,
						"Note list", cmd_info_print),
					
					SHELL_CMD(slave_status, &bus_number_find,
						"Get I2C slave status from bus", cmd_i2c_slave_get_status),
			
					SHELL_CMD(slave_register, &bus_number_find,
						"Register/unregister I2C slave function from bus", cmd_i2c_slave_register),
					
					SHELL_CMD(slave_readsingle, &bus_number_find,
						"I2C slave read 1 message from bus", cmd_i2c_slave_read),

					SHELL_CMD(master_writesingle, &bus_number_find,
						"I2C slave write 1 message from bus", cmd_i2c_slave_write),
					
					SHELL_CMD(case, NULL,
						"I2C slave test case for Ast1030 evb board only", cmd_i2c_slave_test),

					SHELL_CMD(master_write_mctp_cmd, NULL,
						"I2C slave test case for Ast1030 evb board only", cmd_i2c_mctp),

			       SHELL_SUBCMD_SET_END     /* Array terminated. */
			       );

SHELL_CMD_REGISTER(i2c_test, &sub_i2c_slave_cmds, "I2C slave commands", NULL);
