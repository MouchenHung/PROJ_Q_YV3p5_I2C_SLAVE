/*
 * NONE
 */

#include <zephyr.h>
#include <sys/printk.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmsis_os2.h"
#include "hal_i2c.h"
//#include "timer.h"
#include "hal_i2c_slave.h"
#include "i2c_slave_case.h"

/* mcadd */
struct k_thread master_write;
K_KERNEL_STACK_MEMBER(master_write_stack, 1000);
struct k_thread slave_read;
K_KERNEL_STACK_MEMBER(slave_read_stack, 1000);
struct k_thread slave_queue_monitor;
K_KERNEL_STACK_MEMBER(slave_queue_monitor_stack, 1000);

uint8_t case3_master_bus = 0x02;
uint8_t case3_slave_bus = 0x03;
uint8_t case3_slave_addr = 0x30;
uint8_t case3_loop_num = 30;
uint8_t case3_monitor_thread_loop = 60;
uint32_t case3_master_transfer_thread_interval = 500;
uint32_t case3_slave_receive_thread_interval = 1500;
uint8_t case3_stop_flag = 0;

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

static void i2c_slave_queue_monitor_thread(void *arug0, void *arug1, void *arug2){
	ARG_UNUSED(arug0);
	ARG_UNUSED(arug1);
	ARG_UNUSED(arug2);

	while(1){
		if (case3_stop_flag == 2){
			printk("<system> Stop monitor thread.\n");
			break;
		}
		i2c_slave_status_print(case3_slave_bus);
		k_msleep(50);
	}
}

static void i2c_master_transfer_thread(void *arug0, void *arug1, void *arug2){
	ARG_UNUSED(arug0);
	ARG_UNUSED(arug1);
	ARG_UNUSED(arug2);

	I2C_MSG i2c_msg;
	uint8_t retry = 3;
	uint8_t cur_loop = 0;
	
	uint8_t msg_rx_length = 2;
	uint8_t msg_data[] = {0x00, 0x08, 0x05, 0x07};
	uint32_t msg_tx_length = sizeof(msg_data)/sizeof(uint8_t);

	if (msg_tx_length>30){
		printk("<warning> Tx msg length over 30");
		return;
	}
	
	i2c_msg.bus = case3_master_bus;
	i2c_msg.slave_addr = case3_slave_addr;
	i2c_msg.rx_len = msg_rx_length;
	i2c_msg.tx_len = msg_tx_length;
	for (int i=0; i<msg_tx_length; i++){
		i2c_msg.data[i] = msg_data[i];
	}

	while(cur_loop < case3_loop_num){
		if (i2c_msg.data[0] > 0xFF){
			i2c_msg.data[0] = 0;
		}
		i2c_msg.data[0] += 1;

		printk("-------------------- master transfer --------------------\n");
		printk("* i2c master: loop[%d] - Bus[%d] SA[0x%x] msg[0x%x]\n", cur_loop+1, case3_master_bus, case3_slave_addr, i2c_msg.data[0]);
		i2c_master_write(&i2c_msg, retry);
		printk("-------------------- master complete --------------------\n");
		cur_loop++;

		k_msleep(case3_master_transfer_thread_interval);
	}
	case3_stop_flag++;

	return;
}

static void i2c_slave_receive_thread(void *arug0, void *arug1, void *arug2){
	ARG_UNUSED(arug0);
	ARG_UNUSED(arug1);
	ARG_UNUSED(arug2);

	uint16_t rx_len;
	uint8_t ret;
	uint16_t max_slave_read = 30;
	uint8_t *msg = (uint8_t*)malloc(max_slave_read * sizeof(uint8_t));
	uint8_t ipmb_buffer_rx[30];
	uint8_t cur_loop = 0;

	while (cur_loop < case3_loop_num) {
  		k_msleep(case3_slave_receive_thread_interval);
		
		printk("-------------------- slave reading --------------------\n");
		printk("* i2c slave: R[%d] - Bus[%d]\n", cur_loop, case3_slave_bus);

		rx_len = 0;
		ret = i2c_slave_read(case3_slave_bus, msg, max_slave_read, &rx_len);

		if (!ret) {
			memcpy(ipmb_buffer_rx, (uint8_t *)msg, rx_len);
			ipmb_buffer_rx[0] = ipmb_buffer_rx[0] >> 1;
			printk("--> Slave read: ");
			for(int i=0; i<rx_len; i++){
				printk("0x%x ", ipmb_buffer_rx[i]);
			}
			printk("\n-------------------- slave complete --------------------\n");
		} else {
			printk("-------------------- slave complete --------------------\n");
			continue;
		}
		cur_loop++;
	}
	case3_stop_flag++;

	return;
}

static void i2c_transfer_test(uint8_t master_bus, uint8_t slave_bus, uint8_t slave_addr, uint8_t *msg_data, uint32_t msg_tx_length, uint32_t msg_rx_length){
	I2C_MSG i2c_msg;
	uint8_t retry = 3;
	uint8_t ret = 0;
	uint16_t rx_len = 0;
	uint8_t ipmb_buffer_rx[30];
	uint16_t max_slave_read = 30;
	uint8_t msg[max_slave_read];

	if (msg_tx_length>30){
		printk("<warning> Tx msg length over 30");
		return;
	}
	
	i2c_msg.bus = master_bus;
	i2c_msg.slave_addr = slave_addr;
	i2c_msg.rx_len = msg_rx_length;
	i2c_msg.tx_len = msg_tx_length;
	for (int i=0; i<msg_tx_length; i++){
		i2c_msg.data[i] = *(msg_data+i);
	}

	printk("-------------------- testing start --------------------\n");
	printk("Bus[%d] --> Bus[%d]\n", master_bus, slave_bus);

	i2c_master_write(&i2c_msg, retry);

	rx_len = 0;
	ret = i2c_slave_read(slave_bus, msg, max_slave_read, &rx_len);
	if (!ret) {
		memcpy(ipmb_buffer_rx, (uint8_t *)msg, rx_len);
		printk(">> Slave read: ");
		for(int i=0; i<rx_len; i++){
			printk("0x%x ", ipmb_buffer_rx[i]);
		}
		printk("\n");
	} else {
		printk("<error> slave read error with errorcode %d!!!\n", ret);
	}
	
	printk("-------------------- testing end   --------------------\n");
}

void test_case1(void){
	uint8_t master_bus = 0;
	uint8_t slave_bus = 1;

	if (i2c_controller_check(master_bus)){
		printk("< error > Master bus[%d] controller is not enable in device tree!\n", master_bus);
		return;
	}

	if (i2c_controller_check(slave_bus)){
		printk("< error > Slave bus[%d] controller is not enable in device tree!\n", slave_bus);
		return;
	}

	/* Try to get current slave bus1 address */
	struct _i2c_slave_config *cur_cfg = NULL;
	if (i2c_slave_cfg_get(slave_bus, cur_cfg)){
		printk("< error > Slave bus[%d] cfg get failed!\n", slave_bus);
		return;
	}

	if (!cur_cfg->address){
		printk("< error > Slave bus[%d] hasn't init!\n", slave_bus);
		return;
	}

	printk("< system > Start test1.\n\n");
	uint8_t inputdata[] = {0x01, 0x02, 0x03, 0x04, 0x05};
	i2c_transfer_test(master_bus, slave_bus, cur_cfg->address, inputdata, ARRAY_SIZE(inputdata), 2);

	

	printk("< system > End test1.\n\n");
}

void test_case2_1(void){
	int ret;
	uint8_t master_bus = 0;
	uint8_t slave_bus = 1;
	struct _i2c_slave_config *cur_cfg;

	if (i2c_controller_check(master_bus)){
		printk("< error > Master bus[%d] controller is not enable in device tree!\n", master_bus);
		return;
	}

	if (i2c_controller_check(slave_bus)){
		printk("< error > Slave bus[%d] controller is not enable in device tree!\n", slave_bus);
		return;
	}

	printk("< system > Start test2-1.\n\n");
	printk("modify slave in bus 1\n");
	cur_cfg = (struct _i2c_slave_config *)malloc(sizeof(struct _i2c_slave_config));
	cur_cfg->address = 0x44;
	cur_cfg->i2c_msg_count = 0x08;
	ret = i2c_slave_control(slave_bus, cur_cfg, I2C_CONTROL_REGISTER);
	free(cur_cfg);
	if(ret){
		printk("<error> i2c slave register bus[1] failed with errorcode %d!\n", ret);
	}

	uint8_t inputdata[] = {0x01, 0x03, 0x01, 0x04};
	i2c_transfer_test(master_bus, slave_bus, 0x22, inputdata, ARRAY_SIZE(inputdata), 2);

	printk("< system > End test2-1.\n\n");
}

void test_case2_2(void){
	int ret;
	
	uint8_t master_bus = 0;
	uint8_t slave_bus = 1;

	struct _i2c_slave_config *cur_cfg;

	if (i2c_controller_check(master_bus)){
		printk("< error > Master bus[%d] controller is not enable in device tree!\n", master_bus);
		return;
	}

	if (i2c_controller_check(slave_bus)){
		printk("< error > Slave bus[%d] controller is not enable in device tree!\n", slave_bus);
		return;
	}

	master_bus = 2;
	slave_bus = 3;

	if (i2c_controller_check(master_bus)){
		printk("< error > Master bus[%d] controller is not enable in device tree!\n", master_bus);
		return;
	}

	if (i2c_controller_check(slave_bus)){
		printk("< error > Slave bus[%d] controller is not enable in device tree!\n", slave_bus);
		return;
	}

	printk("< system > Start test2-2.\n\n");

	printk("unregister slave in bus 1\n");
	slave_bus = 1;
	ret = i2c_slave_control(slave_bus, NULL, I2C_CONTROL_UNREGISTER);
	if(ret){
		printk("<error> i2c slave unregister bus[1] failed with errorcode %d!\n", ret);
	}

	printk("register slave in bus 3\n");
	master_bus = 2;
	slave_bus = 3;
	cur_cfg = (struct _i2c_slave_config *)malloc(sizeof(struct _i2c_slave_config));
	cur_cfg->address = 0x80;
	cur_cfg->i2c_msg_count = 0x05;
	ret = i2c_slave_control(slave_bus, cur_cfg, I2C_CONTROL_REGISTER);
	free(cur_cfg);
	if (ret){
		printk("<error> i2c slave register bus[3] failed with errorcode %d!\n", ret);
	}

	uint8_t inputdata[] = {0x00, 0x08, 0x05, 0x07};
	i2c_transfer_test(master_bus, slave_bus, 0x40, inputdata, ARRAY_SIZE(inputdata), 2);

	printk("< system > End test2-2.\n\n");
}

void test_case3(void){
	printk("< system > Start test3.\n\n");

	case3_stop_flag = 0;
	
	k_thread_create(&master_write, master_write_stack,
                  K_THREAD_STACK_SIZEOF(master_write_stack),
                  i2c_master_transfer_thread,
                  NULL, NULL, NULL,
                  osPriorityBelowNormal, 0, K_NO_WAIT);
	k_thread_create(&slave_read, slave_read_stack,
                  K_THREAD_STACK_SIZEOF(slave_read_stack),
                  i2c_slave_receive_thread,
                  NULL, NULL, NULL,
                  osPriorityBelowNormal1, 0, K_NO_WAIT);
	k_thread_create(&slave_queue_monitor, slave_queue_monitor_stack,
                  K_THREAD_STACK_SIZEOF(slave_queue_monitor_stack),
                  i2c_slave_queue_monitor_thread,
                  NULL, NULL, NULL,
                  osPriorityBelowNormal2, 0, K_NO_WAIT);

	printk("< system > End test3.\n\n");
}

void test_case4(void){
	int ret;

	uint8_t master_bus = 2;
	uint8_t slave_bus = 3;
	uint16_t test_loop = 500;

	struct _i2c_slave_config *cur_cfg = (struct _i2c_slave_config *)malloc(sizeof(struct _i2c_slave_config));
	
	if (i2c_controller_check(master_bus)){
		printk("< error > Master bus[%d] controller is not enable in device tree!\n", master_bus);
		return;
	}

	if (i2c_controller_check(slave_bus)){
		printk("< error > Slave bus[%d] controller is not enable in device tree!\n", slave_bus);
		return;
	}

	printk("< system > Start test4.\n\n");

	printk("init slave in bus 3\n");
	cur_cfg->address = 0x60;
	cur_cfg->i2c_msg_count = 0x0A;
	ret = i2c_slave_control(slave_bus, cur_cfg, I2C_CONTROL_REGISTER);
	free(cur_cfg);
	if (ret){
		printk("<error> i2c slave register bus[3] failed with errorcode %d!\n", ret);
	}

	uint16_t loop = 0;
	while (loop < test_loop)
	{
		printk("modify slave in bus 3 - loop[%d]\n", loop);
		cur_cfg->address = 0x30;
		cur_cfg->i2c_msg_count = 0x0C;
		ret = i2c_slave_control(slave_bus, cur_cfg, I2C_CONTROL_REGISTER);
		if (ret){
			printk("<error> i2c slave modify bus[3] failed with errorcode %d!\n", ret);
		}
		loop++;
	}
	free(cur_cfg);

	printk("< system > End test4.\n\n");
}

void test_case5(void){
	printk("< system > Start test5.\n\n");
	printk("TODO. Test case for mctp-pldm command\n");
	printk("< system > End test5.\n\n");
}

void test_case6(void){
	int loop = 0, ret = 0;
	int loop_num = 3000;
	uint8_t master_bus, slave_bus;
	struct _i2c_slave_config *cur_cfg = (struct _i2c_slave_config *)malloc(sizeof(struct _i2c_slave_config));
	uint8_t inputdata1[] = {0x00, 0x08, 0x05, 0x07};
	uint8_t inputdata2[] = {0x07, 0x05, 0x08, 0x00};

	printk("< system > Start test6.\n\n");
	while (loop < loop_num)
	{
		printk("#TESTLOOP[%d]\n", loop);

		/* register bus 3 */
		printk("register slave in bus 3\n");
		slave_bus = 3;
		memset(cur_cfg, 0, sizeof(struct _i2c_slave_config));
		cur_cfg->address = 0x80;
		cur_cfg->i2c_msg_count = 0x0A;
		ret = i2c_slave_control(slave_bus, cur_cfg, I2C_CONTROL_REGISTER);
		if (ret){
			printk("<error> i2c slave register bus[%d] failed with errorcode %d!\n", slave_bus, ret);
			goto error;
		}

		/* unregister bus 2 */
		printk("unregister slave in bus 2\n");
		slave_bus = 2;
		ret = i2c_slave_control(slave_bus, NULL, I2C_CONTROL_UNREGISTER);
		if(ret){
			printk("<error> i2c slave unregister bus[%d] failed with errorcode %d!\n", slave_bus, ret);
			goto error;
		}

		/* transfer data */
		printk("transfer data...\n");
		master_bus = 2;
		slave_bus = 3;
		i2c_transfer_test(master_bus, slave_bus, 0x40, inputdata1, ARRAY_SIZE(inputdata1), 2);

		/* unregister bus 3 */
		printk("unregister slave in bus 3\n");
		slave_bus = 3;
		ret = i2c_slave_control(slave_bus, NULL, I2C_CONTROL_UNREGISTER);
		if(ret){
			printk("<error> i2c slave unregister bus[%d] failed with errorcode %d!\n", slave_bus, ret);
			goto error;
		}

		/* register bus 2 */
		printk("register slave in bus 2\n");
		slave_bus = 2;
		memset(cur_cfg, 0, sizeof(struct _i2c_slave_config));
		cur_cfg->address = 0x60;
		cur_cfg->i2c_msg_count = 0x0B;
		ret = i2c_slave_control(slave_bus, cur_cfg, I2C_CONTROL_REGISTER);
		if (ret){
			printk("<error> i2c slave register bus[%d] failed with errorcode %d!\n", slave_bus, ret);
			goto error;
		}

		/* transfer data */
		printk("transfer data...\n");
		master_bus = 3;
		slave_bus = 2;
		i2c_transfer_test(master_bus, slave_bus, 0x30, inputdata2, ARRAY_SIZE(inputdata2), 2);

		printk("\n\n");
		loop++;
	}

error:
	free(cur_cfg);

	printk("< system > End test6.\n\n");
}

void test_case7(void){
	printk("< system > Start test7.\n\n");
	int loop_num = 3000;
	int loop = 0;
	uint8_t status;

	while (loop < loop_num)
	{
		printk("TESTLOOP[%d] - ", loop);
		status = i2c_slave_status_get(3);
		if (status)
			printk("failed!!!, %d\n", status);
		else
			printk("pass!\n");

		loop++;
	}

	printk("< system > End test7.\n\n");
}