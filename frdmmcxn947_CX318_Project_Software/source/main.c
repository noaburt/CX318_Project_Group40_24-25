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
#include <stdio.h>

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Definition for default PWM frequence in hz. */
#ifndef APP_DEFAULT_PWM_FREQUENCY
#define APP_DEFAULT_PWM_FREQUENCY (1000UL)
#endif

static void PWM_DRV_Init3PhPwm(void)
{
    uint16_t deadTimeVal;
    pwm_signal_param_t pwmSignal[2];
    uint32_t pwmSourceClockInHz;
    uint32_t pwmFrequencyInHz = APP_DEFAULT_PWM_FREQUENCY;

    pwmSourceClockInHz = PWM_SRC_CLK_FREQ;

    /* Set deadtime count, we set this to about 650ns */
    deadTimeVal = ((uint64_t)pwmSourceClockInHz * 650) / 1000000000;

    pwmSignal[0].pwmChannel       = kPWM_PwmA;
    pwmSignal[0].level            = kPWM_HighTrue;
    pwmSignal[0].dutyCyclePercent = 50; /* 1 percent dutycycle */
    pwmSignal[0].deadtimeValue    = deadTimeVal;
    pwmSignal[0].faultState       = kPWM_PwmFaultState0;
    pwmSignal[0].pwmchannelenable = true;

    pwmSignal[1].pwmChannel = kPWM_PwmB;
    pwmSignal[1].level      = kPWM_HighTrue;
    /* Dutycycle field of PWM B does not matter as we are running in PWM A complementary mode */
    pwmSignal[1].dutyCyclePercent = 50;
    pwmSignal[1].deadtimeValue    = deadTimeVal;
    pwmSignal[1].faultState       = kPWM_PwmFaultState0;
    pwmSignal[1].pwmchannelenable = true;

    /*********** PWMA_SM0 - phase A, configuration, setup 2 channel as an example ************/
    PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_0, pwmSignal, 2, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz);

    /*********** PWMA_SM1 - phase B configuration, setup PWM A channel only ************/
#ifdef DEMO_PWM_CLOCK_DEVIDER
    PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_1, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz / (1 << DEMO_PWM_CLOCK_DEVIDER));
#else
    PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_1, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz);
#endif

    /*********** PWMA_SM2 - phase C configuration, setup PWM A channel only ************/
#ifdef DEMO_PWM_CLOCK_DEVIDER
    PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_2, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz / (1 << DEMO_PWM_CLOCK_DEVIDER));
#else
    PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_2, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz);
#endif
}



/* Return from main when error without ACTUALLY returning */
void MAIN_CheckErr(status_t result, char* occurrence) {

	if (result == kStatus_Success) { return; }
	PRINTF("PROGRAM FAILED AT: %s with %d\r\n", occurrence, result);

	MAIN_ResetGame();
	//MAIN_PauseIRQs();

	OLED_Reset();

	sprintf(buffer, "PROGRAM FAILED AT:\n%s (%d)", occurrence, result);
	OLED_Print(buffer);

	while (1);

	exit;
}

/* Calculate player score */
int MAIN_CalculateScore() {

	return 100U - playerBuzzes;
}

/* Show score to user */
void MAIN_ShowScore() {
	OLED_Reset();

	sprintf(buffer, "Final Score: %02d!\nWell Done!", displayScore);
	OLED_Print(buffer);
}

/* Show time to user */
void MAIN_ShowTime() {
	OLED_Reset();

	sprintf(buffer, "Time left:\n%02d seconds", playerTime);
	OLED_Print(buffer);
}

/* Show wait message to user */
void MAIN_ShowWait() {
	OLED_Reset();

	sprintf(buffer, "Ready to play!\nLift Hook off Start\nto begin.");
	OLED_Print(buffer);
}

/* Show break message to user */
void MAIN_ShowBreak() {
	OLED_Reset();

	sprintf(buffer, "Relax...");
	OLED_Print(buffer);
}

/* Reset score keeping values */
void MAIN_ResetGame() {
	STATE = STATE_WAIT;

	runTimer = 0U;
	timerFlag = 0U;

	playerBuzzes = 0U;
	playerTime = 60U;
	displayScore = 0U;

	Heartrate_Array_Index = 0;
	Average = 0;
	Heartrate_Array[16] = (uint32_t) {0};

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

		//MAIN_PauseIRQs();

		MAIN_CheckErr(MAX_Read_FIFO((red_buffer+i), (ir_led_buffer+i)), "Max read fifo");  //read from MAX30102 FIFO

		//MAIN_ResumeIRQs();

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
	MAIN_PwmInterrupt();

	PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, ledDutycycle);
	PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_1, kPWM_PwmA, kPWM_SignedCenterAligned, ledDutycycle);

	/* Set the load okay bit for all submodules to load registers from their buffer */
	PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_0 | kPWM_Control_Module_1, true);
	PWM_Delay(ledDelay);


	if (runTimer == 1U) {
		if (timerFlag == 1U) {
			timerFlag = 0U;

			playerTime--;
			MAIN_ShowTime();
		} else {
			timerFlag = 1U;
		}
	}
}

void MAIN_PwmInterrupt() {
	switch (STATE) {

	case STATE_PLAY:
		/* Logic while playing game */

		/* Map heart rate from rest -> MAX to 0% -> 99% duty cycles */
		double hr_factor = (Average * 99U) / MAX_HR;

		ledDutycycle = (uint8_t) ceil(hr_factor);
		if (ledDutycycle > 99U) { ledDutycycle = 99U; }

		break;

	case STATE_BREAK:
		/* Do same as STATE_WAIT, timer is paused */

	case STATE_FINISH:
		/* STATE_FINISH changes to STATE_WAIT once score displayed anyway*/

	case STATE_WAIT:
		/* Set to flash green LEDs, don't spin motor */

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
}

/* GPIO10_IRQn interrupt handler */
/* Change state interrupt */
void GPIO1_INT_0_IRQHANDLER(void) {
  /* Get pin flags 0 */
  uint32_t pin_flags0 = GPIO_GpioGetInterruptChannelFlags(GPIO1, 0U);
  uint8_t NEW_STATE = STATE;

  PRINTF("GPIO\r\n");

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

			runTimer = 0U;
			MAIN_ShowBreak();
			break;
		}

		if (GPIO_PinRead(BOARD_INITPINS_IO_FINISH_GPIO, BOARD_INITPINS_IO_FINISH_GPIO_PIN) == 0) {
			NEW_STATE = STATE_FINISH;
			PRINTF("FINISH from PLAY\r\n");

			runTimer = 0U;
			displayScore = MAIN_CalculateScore();
			MAIN_ShowScore();
			break;
		}

		if (GPIO_PinRead(BOARD_INITPINS_IO_START_GPIO, BOARD_INITPINS_IO_START_GPIO_PIN) == 0) {
			NEW_STATE = STATE_WAIT;
			PRINTF("RESTART from PLAY\r\n");

			MAIN_ResetGame();
			MAIN_ShowWait();
		}

		break;

	case STATE_BREAK:

		if (GPIO_PinRead(BOARD_INITPINS_IO_START_GPIO, BOARD_INITPINS_IO_START_GPIO_PIN) != 0) {
			NEW_STATE = STATE_PLAY;
			PRINTF("PLAY from BREAK\r\n");

			runTimer = 1U;
		}

		break;

	case STATE_WAIT:

		if (GPIO_PinRead(BOARD_INITPINS_IO_START_GPIO, BOARD_INITPINS_IO_START_GPIO_PIN) != 0) {
			NEW_STATE = STATE_PLAY;
			PRINTF("PLAY from WAIT\r\n");

			runTimer = 1U;
		}

		break;

	case STATE_FINISH:
		if (GPIO_PinRead(BOARD_INITPINS_IO_START_GPIO, BOARD_INITPINS_IO_START_GPIO_PIN) == 0) {
			NEW_STATE = STATE_WAIT;
			PRINTF("WAIT from FINISH\r\n");

			MAIN_ShowWait();
		}

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

	/* From PWM example ------------------------------------------------------------------------------------------ */

	/* Enable PWM1 SUB Clockn */
	SYSCON->PWM1SUBCTL |=
		(SYSCON_PWM1SUBCTL_CLK0_EN_MASK | SYSCON_PWM1SUBCTL_CLK1_EN_MASK | SYSCON_PWM1SUBCTL_CLK2_EN_MASK);

	/* Structure of initialize PWM */
	pwm_config_t pwmConfig;
	pwm_fault_param_t faultConfig;
	uint32_t pwmVal = 4;

	PWM_GetDefaultConfig(&pwmConfig);

#ifdef DEMO_PWM_CLOCK_DEVIDER
	pwmConfig.prescale = DEMO_PWM_CLOCK_DEVIDER;
#endif

	/* Use full cycle reload */
	pwmConfig.reloadLogic = kPWM_ReloadPwmFullCycle;
	/* PWM A & PWM B form a complementary PWM pair */
	pwmConfig.pairOperation   = kPWM_ComplementaryPwmA;
	pwmConfig.enableDebugMode = true;

	/* Initialize submodule 0 */
	if (PWM_Init(BOARD_PWM_BASEADDR, kPWM_Module_0, &pwmConfig) == kStatus_Fail)
	{
		MAIN_CheckErr(kStatus_Fail, "PWM initialization failed");
	}

	/* Initialize submodule 1, make it use same counter clock as submodule 0. */
	pwmConfig.clockSource           = kPWM_Submodule0Clock;
	pwmConfig.prescale              = kPWM_Prescale_Divide_1;
	pwmConfig.initializationControl = kPWM_Initialize_MasterSync;
	if (PWM_Init(BOARD_PWM_BASEADDR, kPWM_Module_1, &pwmConfig) == kStatus_Fail)
	{
		MAIN_CheckErr(kStatus_Fail, "PWM initialization failed\n");

	}

	PWM_FaultDefaultConfig(&faultConfig);

#ifdef DEMO_PWM_FAULT_LEVEL
	faultConfig.faultLevel = DEMO_PWM_FAULT_LEVEL;
#endif

	/* Sets up the PWM fault protection */
	PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_0, &faultConfig);
	PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_1, &faultConfig);
	PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_2, &faultConfig);
	PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_3, &faultConfig);

	/* Set PWM fault disable mapping for submodule 0/1/2 */
	PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_0, kPWM_PwmA, kPWM_faultchannel_0,
							 kPWM_FaultDisable_0 | kPWM_FaultDisable_1 | kPWM_FaultDisable_2 | kPWM_FaultDisable_3);
	PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_1, kPWM_PwmA, kPWM_faultchannel_0,
							 kPWM_FaultDisable_0 | kPWM_FaultDisable_1 | kPWM_FaultDisable_2 | kPWM_FaultDisable_3);

	/*
	 * Call the init function with demo configuration.
	 * Recommend to invoke API PWM_SetupPwm after PWM and fault configuration, because reference manual advises to
	 * set OUTEN register after other PWM configurations. But set OUTEN register before MCTRL register is okay.
	 */
	PWM_DRV_Init3PhPwm();

	/* Set the load okay bit for all submodules to load registers from their buffer */
	PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_0 | kPWM_Control_Module_1, true);

	/* Start the PWM generation from Submodules 0, 1 and 2 */
	PWM_StartTimer(BOARD_PWM_BASEADDR, kPWM_Control_Module_0 | kPWM_Control_Module_1);

	/* End of pwm example -----------------------------------------------------------------------------------------------*/

	/* Variables for calculating LED brightness reflecting heart beat */
	uint32_t led_min, led_max, prev_data;
	int i;
	int32_t brightness;

	/* PWM Variables */
	brightnessUp = 1U;

	/* Heart Rate to Led PWM variables */
	ledDutycycle = 10U;


	/* Begin game */

	/* Prepare for reading data */
	brightness = 0;
	led_min = 0x3FFFF;
	led_max = 0;

	/* Buffer length stores 5 seconds of samples at 100s/s */
	ir_buffer_len = 500;

	PRINTF("INITIALISED\r\n");
    OLED_Init();
	OLED_Reset();

	sprintf(buffer, "Waiting...\nPlace Hook on Start");
	OLED_Print(buffer);

	/* Wait until hook is placed on start */
	while (GPIO_PinRead(BOARD_INITPINS_IO_START_GPIO, BOARD_INITPINS_IO_START_GPIO_PIN) == 1) {}

	uint8_t read_first = 1U;

	if (read_first == 1U) {
		/* Read first 500 readings */
		read_first = 0U;
		PRINTF("READING FIRST 500\r\n");
		MAX_ReadFirst(led_min, led_max, i);

		prev_data = red_buffer[i];
	}

	PRINTF("BEGINNING\r\n");
	MAIN_ShowWait();

	/* Game starts in waiting state */
	MAIN_ResetGame();

	while (1) {

		switch (STATE) {

		case STATE_PLAY:
			/* Logic while playing game */

			/* Calculate hr and Sp02 after first 500 samples (5 seconds) */
			maxim_heart_rate_and_oxygen_saturation(ir_led_buffer, ir_buffer_len, red_buffer, &spo2, &spo2_valid, &heart_rate, &hr_valid);

			GPIO_PinWrite(PWM_INITPINS_LED_SELECT_GPIO, PWM_INITPINS_LED_SELECT_GPIO_PIN, SET_RED);

			led_min = 0x3FFFF;
			led_max = 0;

			MAX_ReadAll(led_min, led_max, prev_data, i, brightness);

			maxim_heart_rate_and_oxygen_saturation(
					ir_led_buffer, ir_buffer_len, red_buffer,
					&spo2, &spo2_valid,
					&heart_rate, &hr_valid
			);

			if (hr_valid == 1) {
				if (heart_rate <= MAX_HR && heart_rate >= MIN_HR) { prev_hr = heart_rate; }
			}

			Heartrate_Array[Heartrate_Array_Index] = prev_hr;
			Heartrate_Array_Index = (Heartrate_Array_Index + 1) & 0b1111;

			Average = 0;
			for(int i = 0; i < 16; i++){
				Average += Heartrate_Array[i];
			}
			Average = Average / 16;
			Average = Average;

			float e = exp(1);
			float smoothed = 1;
			smoothed = smoothed / (1 + pow(e,Average));

			PRINTF("Array Index: %i  ---  Average: %i\r\n", Heartrate_Array_Index, (int) ceil(Average));

			if (prev_hr < rest_hr) { rest_hr = prev_hr; }

			PRINTF("HR Valid = %i, HR = %i, Stored HR = %i, Cycle = %d\r\n", hr_valid, heart_rate, prev_hr, ledDutycycle);

			break;

		case STATE_BREAK:
			/* Pause timer and do same as WAIT*/

		case STATE_WAIT:
			/* Set to flash green LEDs */
			GPIO_PinWrite(PWM_INITPINS_LED_SELECT_GPIO, PWM_INITPINS_LED_SELECT_GPIO_PIN, SET_GRN);

			break;

		case STATE_FINISH:
			/* Display score, reset */

			break;

		}

		PWM_Update();
	}
}
