#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_clock.h"
#include "shield_oled.h"
#include <stdio.h>  // for sprintf

int main(void)
{
    /* Init board hardware. */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    SDK_DelayAtLeastUs(1000000, CLOCK_GetFreq(kCLOCK_CoreSysClk)); // delay 1 sec for safety

    OLED_Init();
    OLED_Reset();  // clear display & reset cursor

    OLED_Print("Welcome to the\nWire & Hoop Stress\nManagement Game!");

    PRINTF("Welcome message shown.\r\n");


    SDK_DelayAtLeastUs(3000000, CLOCK_GetFreq(kCLOCK_CoreSysClk)); // wait 3 sec

    // Countdown
    for (int seconds = 60; seconds >= 0; seconds--)
    {
    	OLED_Reset(); // clear screen and reset cursor
        char buffer[32];
        sprintf(buffer, "Time left:\n%02d seconds", seconds);
        OLED_Print(buffer);

        SDK_DelayAtLeastUs(1000000, CLOCK_GetFreq(kCLOCK_CoreSysClk)); // wait 1 sec
    }

    OLED_Reset(); // clear screen
    OLED_Print("You're out of\n time!");

    PRINTF("Countdown finished.\r\n");

    while (1) {}
}
