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
#include "peripherals.h"
#include "fsl_ctimer.h"
#include "app.h"
#include "pin_mux.h"

/* Heart rate sensor includes */
#include "algorithm.h"
#include "MAX30102.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MAX_BRIGHTNESS 255
#define BUZZ_MARGIN 2 // BAD DONT USE THIS NUMBER, ONLY TEST

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ctimer_match0_callback(uint32_t flags);

/* Array of function pointers for callback for each channel */
ctimer_callback_t ctimer_callback_table[] = {
    ctimer_match0_callback, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

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

static int timer_int_flag;
static int timer_counter;
static int prev_buzz_time;
static int buzz_counter;
static ctimer_config_t config;
static ctimer_match_config_t matchConfig;

/*******************************************************************************
 * Code
 ******************************************************************************/

void ctimer_match0_callback(uint32_t flags) {
	timer_int_flag = 1;
}


/* GPIO10_IRQn interrupt handler - RESET_TIMER */
void GPIO0_INT_0_IRQHANDLER(void) {
  /* Get pin flags 0 */
  uint32_t pin_flags0 = GPIO_GpioGetInterruptChannelFlags(GPIO1, 0U);

  timer_counter = 0;
  buzz_counter = 0;
  PRINTF("TIMER RESET\r\n");

  /* Clear pin flags 0 */
  GPIO_GpioClearInterruptChannelFlags(GPIO1, pin_flags0, 0U);

  /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
     Store immediate overlapping exception return operation might vector to incorrect interrupt. */
  #if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
  #endif
}

/* GPIO40_IRQn interrupt handler - BUZZ */
void GPIO4_INT_0_IRQHANDLER(void) {
  /* Get pin flags 0 */
  uint32_t pin_flags0 = GPIO_GpioGetInterruptChannelFlags(GPIO4, 0U);

  /* Allow time for margin of error i.e. bad connection */
  if (timer_counter - prev_buzz_time > BUZZ_MARGIN) {
	  prev_buzz_time = timer_counter;
	  buzz_counter++;
	  PRINTF("BUZZ\r\n");
  }

  /* Clear pin flags 0 */
  GPIO_GpioClearInterruptChannelFlags(GPIO4, pin_flags0, 0U);

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
	timer_counter = 0;
	timer_int_flag = 0;
	buzz_counter = 0;

	BOARD_InitHardware();
	BOARD_InitGPIOInt();

	/* Init ctimer */
	CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1U);
	CLOCK_AttachClk(kFRO_HF_to_CTIMER0);

	CTIMER_GetDefaultConfig(&config);
	CTIMER_Init(CTIMER0_PERIPHERAL, &config);

	matchConfig.enableCounterReset = true;
	matchConfig.enableCounterStop = false;
	matchConfig.matchValue = CTIMER0_TICK_FREQ / 4;
	matchConfig.outControl = kCTIMER_Output_Toggle;
	matchConfig.outPinInitState = true;
	matchConfig.enableInterrupt = true;

	CTIMER_SetupMatch(CTIMER0_PERIPHERAL, CTIMER0_MATCH_0_CHANNEL, &matchConfig);
	CTIMER_RegisterCallBack(CTIMER0_PERIPHERAL, &ctimer_callback_table[0], kCTIMER_MultipleCallback);
	CTIMER_StartTimer(CTIMER0_PERIPHERAL);

	while (1) {

		if (timer_int_flag == 1) {
			PRINTF("TIME: %d, BUZZES: %d\r\n", timer_counter++, buzz_counter);
			timer_int_flag = 0;
		}

	}

}

int MAXmain(void)
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

	initMAX();

	if (resetMAX() != kStatus_Success) { return 1; } // reset the MAX30102

	/* read & clear INT status register */
	if (readFromMAX(0, &dummy) != kStatus_Success) { return 1; }

	/* initialise the MAX30102 */
	if (startMAX() != kStatus_Success) { return 1; }

	/* Prepare for reading data */
	brightness = 0;
	led_min = 0x3FFFF;
	led_max = 0;

	/* Buffer length stores 5 seconds of samples at 100s/s */
	ir_buffer_len = 500;

	/* Read the first 500 samples and determine signal range */
	for (i = 0; i < ir_buffer_len; i++) {

		while (GPIO_PinRead(MAX_INT_GPIO, MAX_INT_GPIO_PIN) == 1);

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
