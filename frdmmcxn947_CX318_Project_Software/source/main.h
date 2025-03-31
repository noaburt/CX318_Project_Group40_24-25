
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

#define MAX_HR 200
#define MIN_HR 40
#define MAX_HR_DELT 100

#define SCTIMER_LED_OUT kSCTIMER_Out_4
#define SCTIMER_MOT_OUT kSCTIMER_Out_0

#define MIN_LED_DUTY 15
#define MAX_MOT_DUTY 50

/* States of state machine */
#define STATE_WAIT 1
#define STATE_PLAY 2
#define STATE_BREAK 3
#define STATE_FINISH 4


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
uint8_t STATE;

uint32_t rest_hr;
uint32_t prev_hr;

uint32_t ir_led_buffer[500]; 	// IR LED sensor data
int32_t ir_buffer_len; 			// IR data length
uint32_t red_buffer[500];		// Red LED sensor data
int32_t spo2; 					// SPo2 value
int8_t spo2_valid; 				// SPo2 calculation validity
int32_t heart_rate; 			// Heart rate value
int8_t hr_valid;				// Heart rate calculation validity
uint8_t dummy;					// General 'dummy' variable

uint8_t sctimerIsrFlag;
uint8_t brightnessUp;
uint8_t ledDutycycle;
uint8_t motorDutycycle;
