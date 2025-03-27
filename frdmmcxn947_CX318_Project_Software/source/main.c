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
#include "clock_config.h"
#include "board.h"

#include "fsl_clock.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MAX_BRIGHTNESS 255

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint32_t ir_led_buffer[500]; 	// IR LED sensor data
int32_t ir_buffer_len; 			// IR data length
uint32_t red_buffer[500];		// Red LED sensor data
int32_t spo2; 					// SPo2 value
int8_t spo2_valid; 				// SPo2 calculation validity
int32_t heart_rate; 			// Heart rate value
int8_t hr_valid;				// Heart rate calculation validity
uint8_t dummy;					// General 'dummy' variable

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Return from main when error without ACTUALLY returning */
void check_error(status_t result, char* occurrence) {

	if (result == kStatus_Success) { return; }
	PRINTF("PROGRAM FAILED AT: %s with %d\r\n", occurrence, result);

	while (1);

	exit;
}

/*!
 * @brief Main function
 */
int main(void)
{
    char ch;

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

    /* Variables for calculating LED brightness reflecting heart beat */
	uint32_t led_min, led_max, prev_data;
	int i;
	int32_t brightness;
	float tmp;

	status_t result;

	MAX_Init();

    SDK_DelayAtLeastUs(1000000, CLOCK_GetFreq(kCLOCK_CoreSysClk));

	check_error(MAX_Reset(), "Max Reset");

    /* Reading REG_INTR_STATUS_1 clears interrupts */
	check_error(MAX_Read(&dummy, REG_INTR_STATUS_1), "Max Read INTR");

    /* Set configuration */
	check_error(MAX_Start(), "Max Start");

	/* Prepare for reading data */
	brightness = 0;
	led_min = 0x3FFFF;
	led_max = 0;

	/* Buffer length stores 5 seconds of samples at 100s/s */
	ir_buffer_len = 500;

	/* Read the first 500 samples and determine signal range */
	for (i = 0; i < ir_buffer_len; i++) {

		while (GPIO_PinRead(MAX_INITIPINS_MAX_INT_GPIO, MAX_INITIPINS_MAX_INT_GPIO_PIN) == 1);

		check_error(MAX_Read_FIFO((red_buffer+i), (ir_led_buffer+i)), "Max read fifo");  //read from MAX30102 FIFO

		/* Update signal mix & max */
		if (red_buffer[i] < led_min) { led_min = red_buffer[i]; }
		if (red_buffer[i] > led_max) { led_max = red_buffer[i]; }

		//PRINTF("red = %d, ir = %d\r\n", red_buffer[i], ir_led_buffer[i]);
	}

	prev_data = red_buffer[i];

	/* Calculate hr and Sp02 after first 500 samples (5 seconds) */
	maxim_heart_rate_and_oxygen_saturation(ir_led_buffer, ir_buffer_len, red_buffer, &spo2, &spo2_valid, &heart_rate, &hr_valid);

	/* Continuously sample, hr & sp02 calculated every 1s*/
	while (1) {

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

			check_error(MAX_Read_FIFO((red_buffer+i), (ir_led_buffer+i)), "Max read fifo");  //read from MAX30102 FIFO

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
		}

		maxim_heart_rate_and_oxygen_saturation(ir_led_buffer, ir_buffer_len, red_buffer, &spo2, &spo2_valid, &heart_rate, &hr_valid);
		PRINTF(
				"HR = %d, HRvalid = %d, SpO2 = %d, SpO2valid = %d\r\n",
				heart_rate, hr_valid, spo2, spo2_valid
		);
	}
}
