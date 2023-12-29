/*
 * process.h
 *
 *  Created on: Nov 12, 2021
 *      Author: igor.dymov
 */

#ifndef INC_PROCESS_H_
#define INC_PROCESS_H_

#include "main.h"
#include "CO_ODinterface.h"
#include "keyboard.h"
#include "led.h"
#include "CANopen.h"
#include "OD.h"
#include "CO_driver_ST32F103.h"

#define NMT_CONTROL                     (CO_NMT_STARTUP_TO_OPERATIONAL | CO_NMT_ERR_ON_ERR_REG | CO_ERR_REG_GENERIC_ERR | CO_ERR_REG_COMMUNICATION)
#define FIRST_HB_TIME                   500
#define SDO_SRV_TIMEOUT_TIME            1000
#define SDO_CLI_TIMEOUT_TIME            500
#define SDO_CLI_BLOCK                   false
#define OD_STATUS_BITS                  NULL

#define ACTIVE  	0x01
#define NOT_ACTIVE  0x00
#define MIN_NODE_ID 0x01
#define MAX_NODE_ID 0x7F
#define MIN_BITRATE 0x00
#define MAX_BITRATE 0x07
#define MAX_COLOR	0x09
#define MAX_BRIGTH  0x3F

#define KEY_STATE_REGISTER  1U
#define K1   0x01
#define K2   0x02
#define K3   0x04
#define K4   0x08
#define K5   0x10
#define K6   0x20
#define K7   0x40
#define K8   0x80

#define keys_state_message 180U

#define KEYBOARD_PERIOD_SUBINDEX    0x01
#define KEYDOWN_DELAY_SUBINDEX      0x02
#define KEYDOWN_HOLD_SUBINDEX       0x03
#define REPEAT_TIME_SUBINDEX        0x04

#define PWM_PERIOD_SUBINDEX		   0x01
#define PWM_DUTY_SUBINDEX		   0x02


void vProcessTask( void * argument );
void vProceesInit( void );
void vCanOpen_Init(void *argument);

ODR_t OD_writeLED(OD_stream_t *stream, const void *buf,
                       OD_size_t count, OD_size_t *countWritten);
#endif /* INC_PROCESS_H_ */
