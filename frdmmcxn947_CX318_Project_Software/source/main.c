/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <max30102.h>
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

    SDK_DelayAtLeastUs(1000000, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    /* Variables for calculating LED brightness reflecting heart beat */
	uint32_t led_min, led_max, prev_data;
	int i;
	int32_t brightness;
	float tmp;

	MAX_Init();
	status_t stop_result = MAX_Reset();

    /* Reading REG_INTR_STATUS_1 clears interrupts */
	status_t stop_result2 = MAX_Read(&dummy, REG_INTR_STATUS_1);

    /* Set configuration */
	status_t stop_result3 = MAX_Start();

    PRINTF("hello world.\r\n");


    while (1)
    {
        ch = GETCHAR();
        PUTCHAR(ch);
    }
}
