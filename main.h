#ifndef __MAIN_H
#define __MAIN_H

#include "gd32f10x.h"
#include <stdio.h>

typedef struct 
{
	 uint32_t can_periph;
}
CAN_HandleTypeDef; 

#define RESTART_DISABLE  0x0001

int main(void);
void vRestartNode( void );
void Error_Handler(void);
#endif