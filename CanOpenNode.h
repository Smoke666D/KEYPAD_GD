/*
 * CanOpenNode.h
 *
 *  Created on: Nov 17, 2021
 *      Author: igor.dymov
 */

#ifndef INC_CANOPENNODE_H_
#define INC_CANOPENNODE_H_

#include "main.h"

void vCanOpenInit(CAN_HandleTypeDef *hcan);
void vCanOpenProcess(void *argument);
void vCanOpenPeriodicProcess(void *argument);

#endif /* INC_CANOPENNODE_H_ */
