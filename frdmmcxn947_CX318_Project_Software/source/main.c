/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <max30102.h>
#include <algorithm.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "peripherals.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_clock.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MAX_BRIGHTNESS 255

#define MAX_HR 180
#define MAX_HR_DELT 50

#define SCTIMER_OUT kSCTIMER_Out_4

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void MAIN_CheckErr(status_t result, char* occurrence);

void MAX_Begin();

void PWM_Delay();
void PWM_Init();
void PWM_Update();


/*******************************************************************************
 * Variables
 ******************************************************************************/
uint32_t rest_hr;
uint32_t prev_hr;
double hr_factor;

uint32_t ir_led_buffer[500]; 	// IR LED sensor data
int32_t ir_buffer_len; 			// IR data length
uint32_t red_buffer[500];		// Red LED sensor data
int32_t spo2; 					// SPo2 value
int8_t spo2_valid; 				// SPo2 calculation validity
int32_t heart_rate; 			// Heart rate value
int8_t hr_valid;				// Heart rate calculation validity
uint8_t dummy;					// General 'dummy' variable

uint8_t sctimerIsrFlag = 0U;
uint8_t brightnessUp = 1U;
uint8_t updatedDutycycle = 10U;

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Return from main when error without ACTUALLY returning */
void MAIN_CheckErr(status_t result, char* occurrence) {

	if (result == kStatus_Success) { return; }
	PRINTF("PROGRAM FAILED AT: %s with %d\r\n", occurrence, result);

	while (1);

	exit;
}

/* Setup and start MAX30102 */
void MAX_Begin() {
	MAX_Init();

	SDK_DelayAtLeastUs(1000000, CLOCK_GetFreq(kCLOCK_CoreSysClk));

	MAIN_CheckErr(MAX_Reset(), "Max Reset");

	/* Reading REG_INTR_STATUS_1 clears interrupts */
	MAIN_CheckErr(MAX_Read(&dummy, REG_INTR_STATUS_1), "Max Read INTR");

	/* Set configuration */
	MAIN_CheckErr(MAX_Start(), "Max Start");
}

void PWM_Delay() {
	volatile uint32_t i = 0U;

	for (i = 0U; i < 80000U; ++i)
	{
		__asm("NOP"); /* delay */
	}
}

/* Use interrupt to update the PWM dutycycle on output */
void PWM_Update() {
	if (sctimerIsrFlag == 1U) {

		/* Disable interrupt to retain current dutycycle for a few seconds */
		SCTIMER_DisableInterrupts(SCT0, (1 << SCT0_pwmEvent[0]));

		sctimerIsrFlag = 0U;

		/* Update PWM duty cycle */
		SCTIMER_UpdatePwmDutycycle(SCT0, SCTIMER_OUT, updatedDutycycle, SCT0_pwmEvent[0]);

		/* Delay to view the updated PWM dutycycle */
		//PWM_Delay();

		/* Enable interrupt flag to update PWM dutycycle */
		SCTIMER_EnableInterrupts(SCT0, (1 << SCT0_pwmEvent[0]));
	}
}

/* SCT0_IRQn interrupt handler */
void SCT0_IRQHANDLER(void) {
  /* Get status flags */
  uint32_t status_flags = SCTIMER_GetStatusFlags(SCT0_PERIPHERAL);

  /* Place your interrupt code here */

//  if (brightnessUp == 1U)
//	{
//	  /* Increase duty cycle until it reach limited value, don't want to go upto 100% duty cycle
//	   * as channel interrupt will not be set for 100%
//	   */
//	  if (++updatedDutycycle >= 99U)
//	  {
//		  updatedDutycycle = 99U;
//		  brightnessUp     = 0U;
//	  }
//	}
//	else
//	{
//	  /* Decrease duty cycle until it reach limited value */
//	  if (--updatedDutycycle == 1U)
//	  {
//		  brightnessUp = 1U;
//	  }
//	}

  /* Map heart rate from rest -> MAX to 0% -> 99% duty cycle */
  hr_factor = (prev_hr / MAX_HR); // ratio of current VALID hr to max heart rate (adding rest hr ratio to map from rest -> MAX)
  updatedDutycycle = 30;

  sctimerIsrFlag = 1U;
  PWM_Update();

  /* Clear status flags */
  SCTIMER_ClearStatusFlags(SCT0_PERIPHERAL, status_flags);

  /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
     Store immediate overlapping exception return operation might vector to incorrect interrupt. */
  #if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
  #endif
}


/*!
 * @brief Main function
 */
int main(void)
{
    char ch;
    hr_factor = 0;

    /* Init board hardware. */
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach TRACECLKDIV to TRACE */
    CLOCK_SetClkDiv(kCLOCK_DivTraceClk, 2U);
    CLOCK_AttachClk(kTRACE_DIV_to_TRACE);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

	BOARD_InitPeripherals();
	/* Enable interrupt flag for event associated with out 4, we use the interrupt to update dutycycle */
	SCTIMER_EnableInterrupts(SCT0, (1 << SCT0_pwmEvent[0]));

	/* Receive notification when event is triggered */
	SCTIMER_SetCallback(SCT0, SCT0_IRQHANDLER, SCT0_pwmEvent[0]);

	MAX_Begin();

    /* Variables for calculating LED brightness reflecting heart beat */
	uint32_t led_min, led_max, prev_data;
	int i;
	int32_t brightness;
	float tmp;

	rest_hr = 75; // Initialise rest hr, can change this

	/* Prepare for reading data */
	brightness = 0;
	led_min = 0x3FFFF;
	led_max = 0;

	/* Buffer length stores 5 seconds of samples at 100s/s */
	ir_buffer_len = 500;

	/* Read the first 500 samples and determine signal range */
	for (i = 0; i < ir_buffer_len; i++) {

		while (GPIO_PinRead(MAX_INITIPINS_MAX_INT_GPIO, MAX_INITIPINS_MAX_INT_GPIO_PIN) == 1);

		MAIN_CheckErr(MAX_Read_FIFO((red_buffer+i), (ir_led_buffer+i)), "Max read fifo");  //read from MAX30102 FIFO

		/* Update signal mix & max */
		if (red_buffer[i] < led_min) { led_min = red_buffer[i]; }
		if (red_buffer[i] > led_max) { led_max = red_buffer[i]; }

	}

	prev_data = red_buffer[i];

	/* Calculate hr and Sp02 after first 500 samples (5 seconds) */
	maxim_heart_rate_and_oxygen_saturation(
			ir_led_buffer, ir_buffer_len, red_buffer,
			&spo2, &spo2_valid,
			&heart_rate, &hr_valid
	);

	while (1) {


		/* Continuously sample, hr & sp02 calculated every 1s */
		led_min = 0x3FFFF;
		led_max = 0;

		/* Dump first 100 sets of samples in memory, shift last 400 sets to top */
		for (i = 100; i < 500; i++) {
			red_buffer[i-100] = red_buffer[i];
			ir_led_buffer[i-100] = ir_led_buffer[i];

			/* Update signal mix & max */
			if (red_buffer[i] < led_min) { led_min = red_buffer[i]; }
			if (red_buffer[i] > led_max) { led_max = red_buffer[i]; }
		}

		/* Take 100 sets of samples before calculating hr */
		for (i = 400; i < 500; i++) {
			prev_data = red_buffer[i-1];

			while (GPIO_PinRead(MAX_INITIPINS_MAX_INT_GPIO, MAX_INITIPINS_MAX_INT_GPIO_PIN) == 1) {}

			MAIN_CheckErr(MAX_Read_FIFO((red_buffer+i), (ir_led_buffer+i)), "Max read fifo");  //read from MAX30102 FIFO

			if (red_buffer[i] > prev_data) {
				tmp = red_buffer[i] - prev_data;
				tmp /= (led_max-led_min);
				tmp *= MAX_BRIGHTNESS;

				brightness -= (int) tmp;
				if (brightness < 0) { brightness = 0; }

			} else {
				tmp = prev_data - red_buffer[i];
				tmp /= (led_max-led_min);
				tmp *= MAX_BRIGHTNESS;

				brightness += (int) tmp;
				if(brightness > MAX_BRIGHTNESS) { brightness = MAX_BRIGHTNESS; }

			}

			/*PRINTF(
					"red = %d, ir = %d, HR = %d, HRvalid = %d, SpO2 = %d, SpO2valid = %d\r\n",
					red_buffer[i], ir_led_buffer[i], heart_rate, hr_valid, spo2, spo2_valid
			);*/

			PWM_Delay();
		}

		maxim_heart_rate_and_oxygen_saturation(
				ir_led_buffer, ir_buffer_len, red_buffer,
				&spo2, &spo2_valid,
				&heart_rate, &hr_valid
		);

		if (hr_valid == 1) {
			if (prev_hr == 0 || abs(heart_rate - prev_hr) < MAX_HR_DELT) {
				if (heart_rate <= MAX_HR) { prev_hr = heart_rate; }
			}
		}

		PRINTF(
				"HR Valid = %d, HR = %d, Stored HR = %d, HR Factor = %%e %%e\r\n", hr_valid, heart_rate, prev_hr, hr_factor, (prev_hr / MAX_HR)
		);

		//PWM_Update();
	}
}
