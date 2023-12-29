/*
 * keyboard.h
 *
 *  Created on: 25 февр. 2020 г.
 *      Author: igor.dymov
 */

#ifndef INC_KEYBOARD_H_
#define INC_KEYBOARD_H_
/*----------------------- Includes -------------------------------------*/
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

/*------------------------ Define --------------------------------------*/
/* Константа определяющая количесвто клавиш, которые обрабатываем драйвер */
#define KEYBOARD_COUNT     8U
#define KEY_READY	         0x0001U
#define KEY_ON_MESSAGE     0x0001U
#define KEY_OFF_MESSAGE    0x0002U
#define KEY_ON_STATE       ( RESET )
#define KEY_OFF_STATE      ( SET )
#define KEY_OFF            0x00U
#define KEY_ON    	       0X01U
#define KEY_ON_REPEAT      0x02U
#define KEY_PEREOD         150U
#define SWITCHONDELAY      30U
#define DefaultDelay       600U
#define DefaultRepeatRate  400U
#define BRAKECODE          0x40U
#define MAKECODE           0x80U


#define kl1_key            0x01U
#define kl2_key            0x02U
#define kl3_key            0x04U
#define kl4_key            0x08U
#define kl5_key            0x10U
#define kl6_key            0x20U
#define kl7_key            0x40U
#define kl8_key            0x80U
#define time_out           0xFFU

#define USE_KEY_TIME_OUT       0

#define  KEY_TIME_OUT      1000U
/*---------------------------- Structures --------------------------------------*/
typedef struct 
{
  uint32_t KeyPort;
  uint16_t      KeyPin;
} xKeyPortStruct;
typedef struct 
{
  unsigned int  KeyCode;
  unsigned char Status;
} KeyEvent;
typedef struct 
{
  unsigned int  sKeyCode;
  unsigned char cStatus;
} xKeyEvent;
/*----------------------------- Functions ------------------------------------*/
void          vKeyboardTask( void  * argument );
void          vSetupKeyboard( void );
void          vKeyboardInit( uint32_t Message );
QueueHandle_t pGetKeyboardQueue( void );
/*----------------------------------------------------------------------------*/
#endif /* INC_KEYBOARD_H_ */
