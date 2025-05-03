
/* File: max30102.c
 *
 * Last updated: 31\03\2025 - extended FIFO almost full value
 *
 */

#include <max30102.h>
#include "fsl_lpi2c.h"
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "peripherals.h"

#include "MCXN947_cm33_core0.h"
#include "fsl_debug_console.h"

/* Initialise the I2C comms */
void MAX_Init(void) {

	uint32_t Clk_Freq = CLOCK_GetFreq(kCLOCK_Fro12M) / CLOCK_GetClkDiv(kCLOCK_DivFlexcom2Clk);

	lpi2c_master_config_t sMasterConfig = {0};
	LPI2C_MasterGetDefaultConfig(&sMasterConfig);
	LPI2C_MasterInit(MAX_I2C, &sMasterConfig, Clk_Freq);

}

/* Set the reset bit in REG_MODE_CONFIG address to reset sensor */
status_t MAX_Reset(void) {

	uint8_t txBuffer = 0x40;

	return MAX_Send(&txBuffer, 1, REG_MODE_CONFIG);
}

/* Set the MAX30102 configurations */
status_t MAX_Start(void) {

	status_t result;

	uint8_t txBuffer[11] = {0xC0, 0x00, 0x00, 0x00, 0x00, 0x40, 0x03, 0x27, 0x24, 0x24, 0x7F};

	result = MAX_Send(&txBuffer[0], 1, REG_INTR_ENABLE_1); // INTR setting
	if(result != kStatus_Success) {
		return result;
	}

	result = MAX_Send(&txBuffer[1], 1, REG_INTR_ENABLE_2);
	if (result != kStatus_Success) {
		return result;
	}

	result = MAX_Send(&txBuffer[2], 1, REG_FIFO_WR_PTR); //FIFO_WR_PTR[4:0]
	if(result != kStatus_Success) {
		return result;
	}

	result = MAX_Send(&txBuffer[3], 1, REG_OVF_COUNTER); //OVF_COUNTER[4:0]
	if(result != kStatus_Success) {
		return result;
	}

	result = MAX_Send(&txBuffer[4], 1, REG_FIFO_RD_PTR); //FIFO_RD_PTR[4:0]
	if(result != kStatus_Success) {
		return result;
	}

	result = MAX_Send(&txBuffer[5], 1, REG_FIFO_CONFIG); //sample avg = 4, fifo rollover=false, fifo almost full = 32
	if(result != kStatus_Success) {
		return result;
	}

	result = MAX_Send(&txBuffer[6], 1, REG_MODE_CONFIG);  //0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
	if(result != kStatus_Success) {
		return result;
	}

	result = MAX_Send(&txBuffer[7], 1, REG_SPO2_CONFIG); // SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)
	if(result != kStatus_Success) {
		return result;
	}

	result = MAX_Send(&txBuffer[8], 1, REG_LED1_PA);  //Choose value for ~ 7mA for LED1
	if(result != kStatus_Success) {
		return result;
	}

	result = MAX_Send(&txBuffer[9], 1, REG_LED2_PA);  // Choose value for ~ 7mA for LED2
	if(result != kStatus_Success) {
		return result;
	}

	return MAX_Send(&txBuffer[10], 1, REG_PILOT_PA);  // Choose value for ~ 25mA for Pilot LED

}

/* Sends the command CD followed by the buffer contents */
status_t MAX_Send(uint8_t* buffer, uint16_t size, uint8_t CD) {

	status_t result;

	lpi2c_master_transfer_t transfer;
	transfer.flags = 0;
	transfer.slaveAddress = MAX_ADDRESS;
	transfer.direction = kLPI2C_Write;
	transfer.subaddress = CD;
	transfer.subaddressSize = 1;
	transfer.data = buffer;
	transfer.dataSize = size;

	result = LPI2C_MasterTransferBlocking(MAX_I2C, &transfer);
	return result;




	result = LPI2C_MasterStart(MAX_I2C, MAX_ADDRESS, kLPI2C_Write);
	if (result != kStatus_Success) {
		PRINTF("SEND START: %d\r\n", result);
		//return result;
	}

	result = LPI2C_MasterSend(MAX_I2C, &CD, 1);
	if (result != kStatus_Success) {
		PRINTF("SEND SEND ADDR: %d\r\n", result);
		//return result;
	}

	result = LPI2C_MasterSend(MAX_I2C, buffer, size);
	if (result != kStatus_Success) {
		PRINTF("SEND SEND DATA: %d\r\n", result);
		//return result;
	}

	result = LPI2C_MasterStop(MAX_I2C);
	if (result != kStatus_Success) {
		PRINTF("SEND STOP: %d\r\n", result);
	}

	return result;

}

/* Sends the command CD followed by reading the address and storing in buffer */
status_t MAX_Read(uint8_t* buffer, uint8_t CD) {

	status_t result;

	lpi2c_master_transfer_t transfer;
	transfer.flags = 0;
	transfer.slaveAddress = MAX_ADDRESS;
	transfer.direction = kLPI2C_Read;
	transfer.subaddress = CD;
	transfer.subaddressSize = 1;
	transfer.data = buffer;
	transfer.dataSize = 1;

	result = LPI2C_MasterTransferBlocking(MAX_I2C, &transfer);
	return result;



	result = LPI2C_MasterStart(MAX_I2C, MAX_ADDRESS, kLPI2C_Write);
	if (result != kStatus_Success) {
		PRINTF("READ START: %d\r\n", result);
		//return result;
	}

	result = LPI2C_MasterSend(MAX_I2C, &CD, 1);
	if (result != kStatus_Success) {
		PRINTF("READ SEND: %d\r\n", result);
		//return result;
	}

	result = LPI2C_MasterRepeatedStart(MAX_I2C, MAX_ADDRESS, kLPI2C_Read);
	if (result != kStatus_Success) {
		PRINTF("READ REPEAT SEND: %d\r\n", result);
		//return result;
	}

	char ch_read_data;

	result = LPI2C_MasterReceive(MAX_I2C, &ch_read_data, 1);
	if (result == kStatus_Success) {
		*buffer = (uint8_t) ch_read_data;
	} else {
		PRINTF("READ READ: %d\r\n", result);
		//return result;
	}

	result = LPI2C_MasterStop(MAX_I2C);
	if (result != kStatus_Success) {
		PRINTF("READ STOP: %d\r\n", result);
	}

	return result;

}

/* Read a set of samples from FIFO */
status_t MAX_Read_FIFO(uint32_t* led_ptr, uint32_t* ir_ptr) {

	status_t result;

	uint32_t tmp;
	uint8_t clear_stat;
	char i2c_data[6];
	uint8_t* i2c_data_ptr = i2c_data;

	*led_ptr = 0;
	*ir_ptr = 0;

	/* Reading status register clears interrupts */
	MAX_Read(&clear_stat, REG_INTR_STATUS_1);
	MAX_Read(&clear_stat, REG_INTR_STATUS_2);

	lpi2c_master_transfer_t transfer;
	transfer.flags = 0;
	transfer.slaveAddress = MAX_ADDRESS;
	transfer.direction = kLPI2C_Read;
	transfer.subaddress = REG_FIFO_DATA;
	transfer.subaddressSize = 1;
	transfer.data = i2c_data_ptr;
	transfer.dataSize = 6;

	result = LPI2C_MasterTransferBlocking(MAX_I2C, &transfer);

	tmp = (unsigned char) i2c_data[0];
	tmp <<= 16;
	*led_ptr += tmp;
	tmp = (unsigned char) i2c_data[1];
	tmp <<= 8;
	*led_ptr += tmp;
	tmp = (unsigned char) i2c_data[2];
	*led_ptr += tmp;

	tmp = (unsigned char) i2c_data[3];
	tmp <<= 16;
	*ir_ptr += tmp;
	tmp = (unsigned char) i2c_data[4];
	tmp <<= 8;
	*ir_ptr += tmp;
	tmp = (unsigned char) i2c_data[5];
	*ir_ptr += tmp;

	*led_ptr &= 0x03FFFF;  //Mask MSB [23:18]
	*ir_ptr &= 0x03FFFF;  //Mask MSB [23:18]

	return result;
}




