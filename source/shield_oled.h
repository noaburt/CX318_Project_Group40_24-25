#ifndef _SHIELD_OLED_H_
#define _SHIELD_OLED_H_

#include "fsl_common.h"

#define SHIELD_OLED_I2C 	(LPI2C2)
#define SHIELD_OLED_LPI2C_MASTER_CLOCK_FREQUENCY CLOCK_GetLPFlexCommClkFreq(2u)

#define OLED_COMMAND 0x00
#define OLED_DATA 0x40
#define OLED_ADDRESS 0x3CU

void OLED_Init(void);
void OLED_Send(uint8_t* buffer, uint16_t size, uint8_t CD);
void OLED_SetPage(uint8_t page);
void OLED_SetSeg(uint8_t seg);
void OLED_FillDisplay(uint8_t data);
void OLED_Print(const char* format, ...);
void OLED_WriteChar(uint8_t character, bool inverted);
void OLED_LineWrap(void);
void OLED_FillPage(uint8_t data);
void OLED_Scroll(uint8_t rows);
void OLED_Reset(void);
void OLED_ResetPrint(void);

extern uint8_t OLED_Logo[1024];
extern char font[256][6];

#endif /* _SHIELD_OLED_H_ */
