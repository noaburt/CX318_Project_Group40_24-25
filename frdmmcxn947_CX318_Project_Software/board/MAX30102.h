#ifndef _MAX30102_H_
#define _MAX30102_H_

#include "fsl_common.h"

/* I2C Interface */
#define MAX_I2C 	(LPI2C2)
#define MAX_ADDRESS 0x57

/* Register Addresses */
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


void MAX_Init(void);
status_t MAX_Reset(void);
status_t MAX_Start(void);

status_t MAX_Send(uint8_t* buffer, uint16_t size, uint8_t CD);
status_t MAX_Read(uint8_t* buffer, uint8_t CD);
status_t MAX_Read_FIFO(uint32_t* led_ptr, uint32_t* ir_ptr);

#endif /* _MAX30102_H_ */
