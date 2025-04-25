/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <main.h>
#include <max30102.h>
#include <algorithm.h>
#include <stdlib.h>
#include <math.h>

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

/* Calculate player score --------------------------------------------------------------------------------------- TODO */
int MAIN_CalculateScore() {

	return 5U;
}

/* Show score to user */
void MAIN_ShowScore() {
	PRINTF("FINAL SCORE: %d\r\n", displayScore);
}

/* Reset score keeping values */
void MAIN_ResetGame() {
	runTimer = 0U;
	playerBuzzes = 0U;
	playerTime = 0U;
	displayScore = 0U;

	motorDelay = PWM_BASE_DELAY;
	ledDelay = PWM_BASE_DELAY;
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

/* Read the first 500 samples and determine signal range */
void MAX_ReadFirst(uint32_t led_min, uint32_t led_max, int i) {
	for (i = 0; i < ir_buffer_len; i++) {

		while (GPIO_PinRead(MAX_INITIPINS_MAX_INT_GPIO, MAX_INITIPINS_MAX_INT_GPIO_PIN) == 1);

		MAIN_CheckErr(MAX_Read_FIFO((red_buffer+i), (ir_led_buffer+i)), "Max read fifo");  //read from MAX30102 FIFO

		/* Update signal mix & max */
		if (red_buffer[i] < led_min) { led_min = red_buffer[i]; }
		if (red_buffer[i] > led_max) { led_max = red_buffer[i]; }

	}
}

/* Sample new readings */
void MAX_ReadAll(uint32_t led_min, uint32_t led_max, uint32_t prev_data, int i, uint32_t brightness) {
	float tmp;

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
	}
}


/* Small delay */
void PWM_Delay(uint32_t delay) {
	volatile uint32_t i = 0U;

	for (i = 0U; i < delay; ++i)
	{
		__asm("NOP"); /* delay */
	}
}

/* Use interrupt to update the PWM dutycycle on output */
void PWM_Update() {

	if (sctimerFlag == 1U) {
		/* Disable interrupt to retain current dutycycle for a few seconds */
		SCTIMER_DisableInterrupts(SCT0, (1 << SCT0_pwmEvent[0]));

		/* Update PWM duty cycles */
		SCTIMER_UpdatePwmDutycycle(SCT0, SCTIMER_LED_OUT, ledDutycycle, SCT0_pwmEvent[0]);
		/* Delay to view the updated PWM dutycycle */
		PWM_Delay(PWM_BASE_DELAY);

		SCTIMER_UpdatePwmDutycycle(SCT0, SCTIMER_MOT_OUT, motorDutycycle, SCT0_pwmEvent[0]);
		/* Delay to view the updated PWM dutycycle */
		PWM_Delay(PWM_BASE_DELAY);

		/* Enable interrupt flag to update PWM dutycycle */
		SCTIMER_EnableInterrupts(SCT0, (1 << SCT0_pwmEvent[0]));

		sctimerFlag = 0U;

		//PRINTF("UPDATE\r\n");
	}
}


/* SCT0_IRQn interrupt handler */
void SCT0_IRQHANDLER(void) {
	/* Get status flags */
	uint32_t status_flags = SCTIMER_GetStatusFlags(SCT0_PERIPHERAL);

	/* Place your interrupt code here */
	sctimerFlag = 1U;

	switch (STATE) {

	case STATE_PLAY:
		/* Logic while playing game */

		/* Map heart rate from rest -> MAX to 0% -> 99% duty cycles */
		double hr_factor = prev_hr - rest_hr;
		hr_factor /= MAX_HR; // ratio of current VALID hr to max heart rate (of range rest -> MAX)

		motorDutycycle = (uint8_t) ceil(hr_factor * 100U);
		ledDutycycle = motorDutycycle;

		if (motorDutycycle > MAX_MOT_DUTY) { motorDutycycle = MAX_MOT_DUTY; }
		if (ledDutycycle > 99U) { ledDutycycle = 99U; }

		break;

	case STATE_BREAK:
		/* Do same as STATE_WAIT, timer is paused */

	case STATE_FINISH:
		/* STATE_FINISH changes to STATE_WAIT once score displayed anyway*/

	case STATE_WAIT:
		/* Set to flash green LEDs, don't spin motor */
		motorDutycycle = 0U;

		if (brightnessUp == 1U) {
			/* Increase duty cycle until it reach limited value, don't want to go upto 100% duty cycle
			* as channel interrupt will not be set for 100%
			*/
			if (++ledDutycycle >= 99U) {
				ledDutycycle = 99U;
				brightnessUp     = 0U;
			}
		} else {
			/* Decrease duty cycle until it reach limited value */
			if (--ledDutycycle == 1U){
				brightnessUp = 1U;
			}
		}

		break;

	}

	/* Clear status flags */
	SCTIMER_ClearStatusFlags(SCT0_PERIPHERAL, status_flags);

	/* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
	 Store immediate overlapping exception return operation might vector to incorrect interrupt. */
  #if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
  #endif
}

/* GPIO10_IRQn interrupt handler */
/* Change state interrupt */
void GPIO1_INT_0_IRQHANDLER(void) {
  /* Get pin flags 0 */
  uint32_t pin_flags0 = GPIO_GpioGetInterruptChannelFlags(GPIO1, 0U);
  uint8_t NEW_STATE = STATE;

  /* Interrupt code here*/
  switch (STATE) {

	case STATE_PLAY:

		if (GPIO_PinRead(BOARD_INITPINS_IO_TRACK_GPIO, BOARD_INITPINS_IO_TRACK_GPIO_PIN) == 0) {
			playerBuzzes++;
			PRINTF("BUZZ\r\n");
		}

		if (GPIO_PinRead(BOARD_INITPINS_IO_BREAK_GPIO, BOARD_INITPINS_IO_BREAK_GPIO_PIN) == 0) {
			NEW_STATE = STATE_BREAK;
			PRINTF("BREAK from PLAY\r\n");
			break;
		}

		if (GPIO_PinRead(BOARD_INITPINS_IO_FINISH_GPIO, BOARD_INITPINS_IO_FINISH_GPIO_PIN) == 0) {
			NEW_STATE = STATE_FINISH;
			PRINTF("FINISH from PLAY\r\n");
			displayScore = MAIN_CalculateScore();
			break;
		}

		if (GPIO_PinRead(BOARD_INITPINS_IO_START_GPIO, BOARD_INITPINS_IO_START_GPIO_PIN) == 0) {
			NEW_STATE = STATE_FINISH;
			PRINTF("RESTART from PLAY\r\n");
		}

		break;

	case STATE_BREAK:

		if (GPIO_PinRead(BOARD_INITPINS_IO_START_GPIO, BOARD_INITPINS_IO_START_GPIO_PIN) != 0) {
			NEW_STATE = STATE_PLAY;
			PRINTF("PLAY from BREAK\r\n");
		}

		break;

	case STATE_WAIT:

		if (GPIO_PinRead(BOARD_INITPINS_IO_START_GPIO, BOARD_INITPINS_IO_START_GPIO_PIN) != 0) {
			NEW_STATE = STATE_PLAY;
			PRINTF("PLAY from WAIT\r\n");
		}

		break;

	case STATE_FINISH:

		NEW_STATE = STATE_WAIT;
		PRINTF("WAIT from FINISH\r\n");
		break;

	}

  STATE = NEW_STATE;

  /* Clear pin flags 0 */
  GPIO_GpioClearInterruptChannelFlags(GPIO1, pin_flags0, 0U);

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

	MAX_Begin();

	/* Variables for calculating LED brightness reflecting heart beat */
	uint32_t led_min, led_max, prev_data;
	int i;
	int32_t brightness;

	/* Timer variables */
	MAIN_ResetGame();

	/* PWM Variables */
	brightnessUp = 1U;
	sctimerFlag = 0U;

	/* Heart Rate to Led PWM variables */
	ledDutycycle = 10U;
	rest_hr = MIN_HR;


	/* Begin game */

	/* Enable interrupt flag for event associated with out 0 and 4, we use the interrupt to update dutycycle */
	SCTIMER_EnableInterrupts(SCT0, (1 << SCT0_pwmEvent[0]));

	/* Receive notification when event is triggered */
	SCTIMER_SetCallback(SCT0, SCT0_IRQHANDLER, SCT0_pwmEvent[0]);

	/* Prepare for reading data */
	brightness = 0;
	led_min = 0x3FFFF;
	led_max = 0;

	/* Buffer length stores 5 seconds of samples at 100s/s */
	ir_buffer_len = 500;
	PRINTF("INITIALISED\r\n");

	/* Wait until hook is placed on start */
	while (GPIO_PinRead(BOARD_INITPINS_IO_START_GPIO, BOARD_INITPINS_IO_START_GPIO_PIN) == 1) {}
	PRINTF("BEGINNING\r\n");

	/* Game starts in waiting state */
	STATE = STATE_WAIT;

	uint8_t FIRST_500 = 0U;

	while (1) {

		switch (STATE) {

		case STATE_PLAY:
			/* Logic while playing game */
			GPIO_PinWrite(PWM_INITPINS_LED_SELECT_GPIO, PWM_INITPINS_LED_SELECT_GPIO_PIN, SET_RED);

			if (FIRST_500 == 0U) {
				PRINTF("READING FIRST 500\r\n");
				MAX_ReadFirst(led_min, led_max, i);

				FIRST_500 = 1U;
				prev_data = red_buffer[i];

				/* Calculate hr and Sp02 after first 500 samples (5 seconds) */
				maxim_heart_rate_and_oxygen_saturation(ir_led_buffer, ir_buffer_len, red_buffer, &spo2, &spo2_valid, &heart_rate, &hr_valid);
			}

			led_min = 0x3FFFF;
			led_max = 0;
			MAX_ReadAll(led_min, led_max, prev_data, i, brightness);
			maxim_heart_rate_and_oxygen_saturation(
					ir_led_buffer, ir_buffer_len, red_buffer,
					&spo2, &spo2_valid,
					&heart_rate, &hr_valid
			);

			if (hr_valid == 1) {
				if (prev_hr == 0 || abs(heart_rate - prev_hr) < MAX_HR_DELT) {
					if (heart_rate <= MAX_HR && heart_rate >= MIN_HR) { prev_hr = heart_rate; }
				}
			}

			if (prev_hr < rest_hr) { rest_hr = prev_hr; }

			PRINTF("HR Valid = %i, HR = %i, Stored HR = %i, Cycle = %d\r\n", hr_valid, heart_rate, prev_hr, ledDutycycle);
			break;

		case STATE_BREAK:
			/* Pause timer and do same as WAIT*/
			runTimer = 0U;

		case STATE_WAIT:
			/* Set to flash green LEDs */
			//PRINTF("WAIT\r\n");
			GPIO_PinWrite(PWM_INITPINS_LED_SELECT_GPIO, PWM_INITPINS_LED_SELECT_GPIO_PIN, SET_GRN);
			break;

		case STATE_FINISH:
			/* Display score, reset */
			MAIN_ShowScore();
			MAIN_ResetGame();
			break;

		}

		PWM_Update();
	}
}
