#ifndef I2C_SLAVE_CASE_H
#define I2C_SLAVE_CASE_H

#define EVB_BOARD 1
#define MCTP_CMD_SUPPORT 0

#if EVB_BOARD == 1
void test_case1(void);
void test_case2_1(void);
void test_case2_2(void);
void test_case3(void);
void test_case4(void);
void test_case6(void);
void test_case7(void);
#if MCTP_CMD_SUPPORT == 1
#include "mctp.h"
#include "pldm.h"
#define MCTP_SMBUS_NUM 2

typedef struct _mctp_smbus_port {
	mctp *mctp_inst;
	mctp_medium_conf conf;
} mctp_smbus_port;
extern mctp_smbus_port smbus_port[MCTP_SMBUS_NUM];

void test_case5(void);

#endif

#endif

#endif
