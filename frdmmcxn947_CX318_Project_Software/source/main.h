
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "peripherals.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_clock.h"
#include "shield_oled.h"
#include "fsl_pwm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* HR Sensor*/
#define MAX_BRIGHTNESS 255

#define MAX_HR 500
#define MIN_HR 40

/* SCTimer & PWM */
#define PWM_BASE_DELAY 40000

#define SCTIMER_LED_OUT kSCTIMER_Out_4
#define SCTIMER_MOT_OUT kSCTIMER_Out_0

#define MAX_MOT_DUTY 50U

/* LED */
#define SET_GRN 0
#define SET_RED 1

/* States of state machine */
#define STATE_WAIT 1
#define STATE_PLAY 2
#define STATE_BREAK 3
#define STATE_FINISH 4

/* From PWM example */
#define BOARD_PWM_BASEADDR        PWM1
#define PWM_SRC_CLK_FREQ          CLOCK_GetFreq(kCLOCK_BusClk)
#define DEMO_PWM_FAULT_LEVEL      true
#define APP_DEFAULT_PWM_FREQUENCY (10000UL)


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void MAIN_CheckErr(status_t result, char* occurrence);
int MAIN_CalculateScore();
void MAIN_ShowScore();
void MAIN_ShowTime();
void MAIN_ShowWait();
void MAIN_ShowBreak();

void MAIN_ResetGame();

void MAIN_PwmInterrupt();

void MAX_Begin();
void MAX_ReadFirst(uint32_t led_min, uint32_t led_max, int i);
void MAX_ReadAll(uint32_t led_min, uint32_t led_max, uint32_t prev_data, int i, uint32_t brightness);

void PWM_Delay(uint32_t delay);
void PWM_Update();

uint32_t Heartrate_Array[16] = {0};
int Heartrate_Array_Index;
float Average;
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t STATE;
uint8_t NEW_STATE;

uint32_t rest_hr;
uint32_t prev_hr;

char buffer[32]; // OLED

uint32_t ir_led_buffer[500]; 	// IR LED sensor data
int32_t ir_buffer_len; 			// IR data length
uint32_t red_buffer[500];		// Red LED sensor data
int32_t spo2; 					// SPo2 value
int8_t spo2_valid; 				// SPo2 calculation validity
int32_t heart_rate; 			// Heart rate value
int8_t hr_valid;				// Heart rate calculation validity
uint8_t dummy;					// General 'dummy' variable

uint8_t brightnessUp;
uint8_t gpioFlag;

uint8_t ledDutycycle;
uint32_t ledDelay;
uint32_t motorDelay;

uint8_t runTimer;
uint8_t timerFlag;
uint32_t playerTime;
uint32_t playerBuzzes;
uint32_t displayScore;



