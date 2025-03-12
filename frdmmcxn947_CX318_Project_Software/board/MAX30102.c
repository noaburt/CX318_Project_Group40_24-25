
/* MAX30102 file - implementations for communicating with MAXREFDES117 board */
/* Slightly altered version of MAX30102.c from sample code */

#include "MAX30102.h"
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "fsl_lpi2c.h"

int sendToMAX(uint8_t reg_addr, uint8_t reg_data)
/**
* \brief        Write a value to a MAX30102 register
* \par          Details
*               This function writes a value to a MAX30102 register
*
* \param[in]    reg_addr    - register address
* \param[in]    reg_data    - register data
*
* \retval       kStatus type
*/
{
	LPI2C_MasterStart(I2C_MAX, MAX_WRITE_ADDR, kLPI2C_Write);

	char i2c_data[2] = {reg_addr, reg_data};

	int result = LPI2C_MasterSend(I2C_MAX, i2c_data, 2);
	LPI2C_MasterStop(I2C_MAX);

	return result;
}

int readFromMAX(uint8_t read_addr, uint8_t *read_data)
/**
* \brief        Read a MAX30102 register
* \par          Details
*               This function reads a MAX30102 register
*
* \param[in]    read_addr    - register address
* \param[out]   read_data    - pointer that stores the register data
*
* \retval       kStatus type
*/
{
	//if (LPI2C_MasterRepeatedStart(LPI2C0, I2C_WRITE_ADDR, kLPI2C_Write) != kStatus_Success) {
	//	return kStatus_Fail;
	//}

	LPI2C_MasterStart(I2C_MAX, MAX_WRITE_ADDR, kLPI2C_Write);

	char ch_i2c_data = read_addr;

	if (LPI2C_MasterSend(I2C_MAX, &ch_i2c_data, 1) != kStatus_Success) {
	    return kStatus_Fail;
	}

	//if (LPI2C_MasterStart(LPI2C0, I2C_READ_ADDR, kLPI2C_Read) != kStatus_Success) {
	//	return kStatus_Fail;
	//}

	int result = kStatus_Fail;

	LPI2C_MasterStart(I2C_MAX, MAX_READ_ADDR, kLPI2C_Read);

	if (LPI2C_MasterReceive(I2C_MAX, &ch_i2c_data, 1) == kStatus_Success) {
		*read_data = (uint8_t) ch_i2c_data;
		result = kStatus_Success;
	}

	LPI2C_MasterStop(I2C_MAX);

	return result;
}

int initMAX()
/**
* \brief        Initialises the I2C interface for MAX30102
* \par          Details
*               This function initialises the I2C interface for MAX30102
*
* \param        None
*
* \retval       None
*/
{
	lpi2c_master_config_t sMasterConfig = {0};
	LPI2C_MasterGetDefaultConfig(&sMasterConfig);
	LPI2C_MasterInit(I2C_MAX, &sMasterConfig, LPI2C_MASTER_CLOCK_FREQUENCY);

	return startMAX();
}

int startMAX()
/**
* \brief        Puts the MAX30102 into Start configuration
* \par          Details
*               This function puts the MAX30102 into Start position
*
* \param        None
*
* \retval       true on success
*/
{
  if(sendToMAX(REG_INTR_ENABLE_1, 0xc0) != kStatus_Success) {// INTR setting
    return kStatus_Fail;
  }

  if(sendToMAX(REG_INTR_ENABLE_2, 0x00) != kStatus_Success) {
    return kStatus_Fail;
  }

  if(sendToMAX(REG_FIFO_WR_PTR, 0x00) != kStatus_Success) { //FIFO_WR_PTR[4:0]
    return kStatus_Fail;
  }

  if(sendToMAX(REG_OVF_COUNTER, 0x00) != kStatus_Success) { //OVF_COUNTER[4:0]
    return kStatus_Fail;
  }

  if(sendToMAX(REG_FIFO_RD_PTR, 0x00) != kStatus_Success) { //FIFO_RD_PTR[4:0]
    return kStatus_Fail;
  }

  if(sendToMAX(REG_FIFO_CONFIG, 0x0f) != kStatus_Success) { //sample avg = 1, fifo rollover=false, fifo almost full = 17
    return kStatus_Fail;
  }

  if(sendToMAX(REG_MODE_CONFIG, 0x03) != kStatus_Success) {  //0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
    return kStatus_Fail;
  }

  if(sendToMAX(REG_SPO2_CONFIG, 0x27) != kStatus_Success) { // SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)
    return kStatus_Fail;
  }


  if(sendToMAX(REG_LED1_PA, 0x24) != kStatus_Success) {  //Choose value for ~ 7mA for LED1
    return kStatus_Fail;
  }

  if(sendToMAX(REG_LED2_PA, 0x24) != kStatus_Success) {  // Choose value for ~ 7mA for LED2
    return kStatus_Fail;
  }

  if(sendToMAX(REG_PILOT_PA, 0x7f) != kStatus_Success) {  // Choose value for ~ 25mA for Pilot LED
    return kStatus_Fail;
  }

  return kStatus_Success;
}

int readFifoMAX(uint32_t *read_led_ptr, uint32_t *read_ir_ptr)
/**
* \brief        Read a set of samples from the MAX30102 FIFO register
* \par          Details
*               This function reads a set of samples from the MAX30102 FIFO register
*
* \param[out]   *read_led_ptr   - pointer that stores the red LED reading data
* \param[out]   *read_ir_ptr    - pointer that stores the IR LED reading data
*
* \retval       kStatus type
*/
{
  uint32_t temp;
  unsigned char read_temp;
  char i2c_data[6];

  *read_led_ptr = 0;
  *read_ir_ptr = 0;


  /* read and clear status register */
  readFromMAX(REG_INTR_STATUS_1, &read_temp);
  readFromMAX(REG_INTR_STATUS_2, &read_temp);

  i2c_data[0] = REG_FIFO_DATA;

  int result = kStatus_Success;

  LPI2C_MasterStart(I2C_MAX, MAX_WRITE_ADDR, kLPI2C_Write);
  result = LPI2C_MasterSend(I2C_MAX, i2c_data, 1);
  LPI2C_MasterStop(I2C_MAX);

  if (result != kStatus_Success) { return result; }

  LPI2C_MasterStart(I2C_MAX, MAX_READ_ADDR, kLPI2C_Read);
  result = LPI2C_MasterReceive(I2C_MAX, i2c_data, 6);
  LPI2C_MasterStop(I2C_MAX);

  if (result != kStatus_Success) { return result; }

  temp = (unsigned char) i2c_data[0];
  temp <<= 16;
  *read_led_ptr += temp;
  temp = (unsigned char) i2c_data[1];
  temp <<= 8;
  *read_led_ptr += temp;
  temp = (unsigned char) i2c_data[2];
  *read_led_ptr += temp;

  temp = (unsigned char) i2c_data[3];
  temp <<= 16;
  *read_ir_ptr += temp;
  temp = (unsigned char) i2c_data[4];
  temp <<= 8;
  *read_ir_ptr += temp;
  temp = (unsigned char) i2c_data[5];
  *read_ir_ptr += temp;

  *read_led_ptr &= 0x03FFFF;  //Mask MSB [23:18]
  *read_ir_ptr &= 0x03FFFF;  //Mask MSB [23:18]

  return kStatus_Success;
}

int resetMAX()
/**
* \brief        Reset the MAX30102
* \par          Details
*               This function resets the MAX30102
*
* \param        None
*
* \retval       true on success
*/
{

    return sendToMAX(REG_MODE_CONFIG, 0x40);
}
