/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Board includes */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "app.h"
#include "pin_mux.h"

/* Heart rate sensor includes */
#include "algorithm.h"
#include "MAX30102.h"

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
/*!
 * @brief Main function
 */
int main(void)
{
    char ch;

    /* Init board hardware. */
    BOARD_InitHardware();

    SDK_DelayAtLeastUs(1000000, CLOCK_GetFreq( kCLOCK_CoreSysClk ));

    /* Variables for calculating LED brightness reflecting heart beat */
	uint32_t led_min, led_max, prev_data;
	int i;
	int32_t brightness;
	float temp;

	if (resetMAX() != kStatus_Success) { return 1; } // reset the MAX30102

	/* read & clear status register */
	if (readFromMAX(0, &dummy) != kStatus_Success) { return 1; }

	/* initialise the MAX30102 */
	if (initMAX() != kStatus_Success) { return 1; }

	/* Prepare for reading data */
	brightness = 0;
	led_min = 0x3FFFF;
	led_max = 0;

	/* Buffer length stores 5 seconds of samples at 100s/s */
	ir_buffer_len = 500;

	/* Read the first 500 samples and determine signal range */
	for (i = 0; i < ir_buffer_len; i++) {

		while (GPIO_PinRead(MAX_INT_GPIO, MAX_INT_GPIO_PIN) == 1) {}

		readFifoMAX((red_buffer+i), (ir_led_buffer+i));  //read from MAX30102 FIFO

		/* Update signal mix & max */
		if (red_buffer[i] < led_min) { led_min = red_buffer[i]; }
		if (red_buffer[i] > led_max) { led_max = red_buffer[i]; }

		PRINTF("red = %i, ir = %i\r\n", red_buffer[i], ir_led_buffer[i]);
	}

	prev_data = red_buffer[i];

	/* Calculate hr and Sp02 after first 500 samples (5 seconds) */
	maxim_heart_rate_and_oxygen_saturation(
			ir_led_buffer, ir_buffer_len, red_buffer,
			&spo2, &spo2_valid,
			&heart_rate, &hr_valid
	);

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

			while (GPIO_PinRead(MAX_INT_GPIO, MAX_INT_GPIO_PIN) == 1) {}

			readFifoMAX((red_buffer+i), (ir_led_buffer+i));

			if (red_buffer[i] > prev_data) {
				temp = red_buffer[i] - prev_data;
				temp /= (led_max-led_min);
				temp *= MAX_BRIGHTNESS;

				brightness -= (int) temp;
				if (brightness < 0) { brightness = 0; }

			} else {
				temp = prev_data - red_buffer[i];
				temp /= (led_max-led_min);
				temp *= MAX_BRIGHTNESS;

				brightness += (int) temp;
				if(brightness > MAX_BRIGHTNESS) { brightness = MAX_BRIGHTNESS; }

			}

			// WRITE TO LEDs

			PRINTF(
					"red = %i, ir = %i, HR = %i, HRvalid = %i, SpO2 = %i, SpO2valid = %i\r\n",
					red_buffer, ir_led_buffer, heart_rate, hr_valid, spo2, spo2_valid
			);

			maxim_heart_rate_and_oxygen_saturation(
					ir_led_buffer, ir_buffer_len, red_buffer,
					&spo2, &spo2_valid,
					&heart_rate, &hr_valid
			);
		}
	}
}
