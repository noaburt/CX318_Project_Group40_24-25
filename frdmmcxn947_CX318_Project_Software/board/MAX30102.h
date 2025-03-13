
/* MAX30102 header file - addresses & functions for communicating with MAXREFDES117 board */
/* Slightly altered version of MAX30102.h from sample code */

#include <stdint.h>

/* I2C constants */
#define EXAMPLE_I2C_MASTER_BASE (LPI2C2_BASE)
#define LPI2C_MASTER_CLOCK_FREQUENCY CLOCK_GetLPFlexCommClkFreq(2u)
#define EXAMPLE_I2C_MASTER ((LPI2C_Type *)EXAMPLE_I2C_MASTER_BASE)
#define I2C_MAX LPI2C2

/* read / write registers */
#define MAX_READ_ADDR 0xAF
#define MAX_WRITE_ADDR 0xAE

/* register addresses */
#define REG_INTR_STATUS_1 0x00
#define REG_INTR_STATUS_2 0x01
#define REG_INTR_ENABLE_1 0x02
#define REG_INTR_ENABLE_2 0x03
#define REG_FIFO_WR_PTR 0x04
#define REG_OVF_COUNTER 0x05
#define REG_FIFO_RD_PTR 0x06
#define REG_FIFO_DATA 0x07
#define REG_FIFO_CONFIG 0x08
#define REG_MODE_CONFIG 0x09
#define REG_SPO2_CONFIG 0x0A
#define REG_LED1_PA 0x0C
#define REG_LED2_PA 0x0D
#define REG_PILOT_PA 0x10
#define REG_MULTI_LED_CTRL1 0x11
#define REG_MULTI_LED_CTRL2 0x12
#define REG_TEMP_INTR 0x1F
#define REG_TEMP_FRAC 0x20
#define REG_TEMP_CONFIG 0x21
#define REG_PROX_INT_THRESH 0x30
#define REG_REV_ID 0xFE
#define REG_PART_ID 0xFF

int initMAX();
int startMAX();
int readFifoMAX(uint32_t *read_led_ptr, uint32_t *read_ir_ptr);
int sendToMAX(uint8_t reg_addr, uint8_t* reg_data_ptr);
int readFromMAX(uint8_t read_addr, uint8_t* read_data);
int resetMAX();
