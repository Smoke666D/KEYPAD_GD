/*
 * flash_data.h
 *
 *  Created on: Dec 1, 2021
 *      Author: igor.dymov
 */

#include "main.h"


#define FLASH_DATA_ADR 0x800fC00
#define CODE_ADR       		       0x00
#define BITRATE_ADR    			   0x01
#define NODE_ID_ADR    			   0x02
#define NMT_STATE_ADR  	    	   0x03
#define DEF_LED_BRIGTH_ADR  	   0x04
#define DEF_BL_BRIGTH_ADR   	   0x05
#define DEF_BL_COLOR_ADR    	   0x06
#define NMT_START_MESSAGE   	   0x07
#define KEYBOARD_PERIOD_ADRRES     0x08
#define KEYDOWN_DELAY_ADRRES       0x09
#define KEYDOWN_HOLD_ADDRESS       0x0A
#define REPEAT_TIME_ADDRESS        0x0B
#define PWM_PERIOD_ADDRESS		   0x0C
#define PWM_DUTY_ADDRESS		   0x0E


#define REG_SIZE      		PWM_DUTY_ADDRESS	+ 2

#define VALID_CODE   	0x67
#define FLASH_SIZE  	0x08007FFFU
#define APP_ADDRESS    	0x08008000U

void vFDInit( void );
void * cgetREGAdr(uint8_t adr);
uint16_t vGetBitrate( void );
uint8_t vGetNodeId( void );
uint16_t vFDGetNMTState( void );
uint8_t vFDGetRegState(uint8_t adr);
void vFDSetRegState(uint8_t adr, uint8_t state);
uint8_t vFDGetStartMessage( void );
void vFDSetRegState16(uint8_t adr, uint16_t state);

