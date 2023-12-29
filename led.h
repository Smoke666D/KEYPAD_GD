/*
 * led.h
 *
 *  Created on: Nov 12, 2021
 *      Author: igor.dymov
 */

#ifndef INC_LED_H_
#define INC_LED_H_


#include "main.h"
//#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "flash_data.h"




#define BUTTON_COUNT 8

#define  MAX_BRIGTH 0x3F
#define  MAX_DATA 0xFF
#define  MAX_BRIGTH_COUNTER MAX_BRIGTH *3
#define  OFF 0x00

#define LATCH_DEALY 	100U
#define SPI_PACKET_SIZE 3U
#define SPI_TIMEOUT		100U

	#define RED_COLOR 	1U
	#define GREEN_COLOR 2U
	#define BLUE_COLOR  3U


#define RED		     0x01
#define GREEN	     0x02
#define BLUE	     0x03
#define YELLOW       0x04
#define CYAN		 0x05
#define VIOLET		 0x06
#define WHITE		 0x07
#define AMBER        0x08
#define YELLOW_GREEN 0x09


#define SET_LED_ON_RED		0x01
#define SET_LED_ON_GREEN	0x02
#define SET_LED_ON_BLUE 	0x03
#define SET_LED_BLINK_RED	0x04
#define SET_LED_BLINK_GREEN	0x05
#define SET_LED_BLINK_BLUE 	0x06

#define DISABLE    0x00
#define FULL_SHOW  0x01
#define FLASH_SHOW 0x02

void vSetBackLigthColor(uint8_t color);
void vSetLedBrigth(uint8_t brigth);
void vSetLedOn(uint8_t Color,uint8_t State);
void vSetLedBlink(uint8_t Color,uint8_t State);
void vSetBrigth(uint8_t brigth);
void vLedInit(uint32_t htim);
void vStartLed( void );
void vSetBackLigth(uint8_t brigth);
void vLedDriverStart(void);
void vLedProcess( void );
#endif /* INC_LED_H_ */
