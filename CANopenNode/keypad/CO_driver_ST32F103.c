/*
 * CAN module object for generic microcontroller.
 *
 * This file is a template for other microcontrollers.
 *
 * @file        CO_driver.c
 * @ingroup     CO_driver
 * @author      Janez Paternoster
 * @copyright   2004 - 2020 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <https://github.com/CANopenNode/CANopenNode>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "301/CO_driver.h"
#include "CO_driver_ST32F103.h"
#include "init.h"

/* CAN masks for identifiers */
#define CANID_MASK                              0x07FF  /*!< CAN standard ID mask */
#define FLAG_RTR                                0x8000  /*!< RTR flag, part of identifier */
/* Mutex for atomic access */
static osMutexId_t co_mutex;

/* Semaphore for main app thread synchronization */
osSemaphoreId_t co_drv_app_thread_sync_semaphore;

/* Semaphore for periodic thread synchronization */
osSemaphoreId_t co_drv_periodic_thread_sync_semaphore;
/* Local CAN module object */
static CO_CANmodule_t* CANModule_local = NULL;  /* Local instance of global CAN module */

uint8_t co_drv_create_os_objects(void) {
    /* Create new mutex for OS context */
    if (co_mutex == NULL) {
        const osMutexAttr_t attr = {
            .attr_bits = osMutexRecursive,
            .name = "co"
        };
        co_mutex = osMutexNew(&attr);
    }

    /* Semaphore for main app thread synchronization */
    if (co_drv_app_thread_sync_semaphore == NULL) {
        const osSemaphoreAttr_t attr = {
                .name = "co_app_thread_sync"
        };
        co_drv_app_thread_sync_semaphore = osSemaphoreNew(1, 1, &attr);
    }

    /* Semaphore for periodic thread synchronization */
    if (co_drv_periodic_thread_sync_semaphore == NULL) {
        const osSemaphoreAttr_t attr = {
                .name = "co_periodic_thread_sync"
        };
        co_drv_periodic_thread_sync_semaphore = osSemaphoreNew(1, 1, &attr);
    }

    return 1;
}

/**
 * \brief           Lock mutex or wait to be available
 * \return          `1` on success, `0` otherwise
 */
uint8_t
co_drv_mutex_lock(void) {
    return osMutexAcquire(co_mutex, osWaitForever) == osOK;
}

/**
 * \brief           Release previously locked mutex
 * \return          `1` on success, `0` otherwise
 */
uint8_t
co_drv_mutex_unlock(void) {
    return osMutexRelease(co_mutex) == osOK;
}

/******************************************************************************/
void CO_CANsetConfigurationMode(void *CANptr){
    /* Put CAN module in configuration mode */
	
	if (CANptr != NULL)
	{
		
		can_working_mode_set(CAN0, CAN_MODE_INITIALIZE);
  }
}

/******************************************************************************/
void CO_CANsetNormalMode(CO_CANmodule_t *CANmodule){
    /* Put CAN module in normal mode */
	     /* Put CAN module in normal mode */
	
	if (CANmodule->CANptr != NULL)
	{
		  
		  if (can_working_mode_set(CAN0,CAN_MODE_NORMAL ) == SUCCESS )
		   CANmodule->CANnormal = true;
	}
}


/******************************************************************************/
CO_ReturnError_t CO_CANmodule_init(
        CO_CANmodule_t         *CANmodule,
        void                   *CANptr,
        CO_CANrx_t              rxArray[],
        uint16_t                rxSize,
        CO_CANtx_t              txArray[],
        uint16_t                txSize,
        uint16_t                CANbitRate)
{
	
	
	
	 uint16_t i;
    can_filter_parameter_struct     can_filter;	
	  CAN_HandleTypeDef * hcan = CANptr;
	
	
    /* verify arguments */
    if(CANmodule==NULL || rxArray==NULL || txArray==NULL){
        return CO_ERROR_ILLEGAL_ARGUMENT;
    }
    CANModule_local = CANmodule;
    /* Configure object variables */
    CANmodule->CANptr = CANptr;
    CANmodule->rxArray = rxArray;
    CANmodule->rxSize = rxSize;
    CANmodule->txArray = txArray;
    CANmodule->txSize = txSize;
    CANmodule->CANerrorStatus = 0;
    CANmodule->CANnormal = false;
    CANmodule->useCANrxFilters = (rxSize <= 32U) ? true : false;/* microcontroller dependent */
    CANmodule->bufferInhibitFlag = false;
    CANmodule->firstCANtxMessage = true;
    CANmodule->CANtxCount = 0U;
    CANmodule->errOld = 0U;

    for(i=0U; i<rxSize; i++){
        rxArray[i].ident = 0U;
        rxArray[i].mask = 0xFFFFU;
        rxArray[i].object = NULL;
        rxArray[i].CANrx_callback = NULL;
    }
    for(i=0U; i<txSize; i++){
        txArray[i].bufferFull = false;
    }

       /* Configure CAN timing */
        MX_CAN_Init( CAN0, CANbitRate);
    		
				/*
				Настройка аппаратных фильтров пакетов
				*/
			  can_filter.filter_number = 0;

				/* initialize filter */    
				can_filter.filter_mode = CAN_FILTERMODE_LIST;
				can_filter.filter_bits = CAN_FILTERBITS_16BIT;
				can_filter.filter_list_high =  ( 0x200 | vGetNodeId() ) <<5;
				can_filter.filter_list_low = ( 0x180 | vGetNodeId() ) <<5;
				can_filter.filter_mask_high = ( 0x400 | vGetNodeId() ) <<5;
				can_filter.filter_mask_low = ( 0x300 | vGetNodeId() ) <<5; 
				can_filter.filter_fifo_number = CAN_FIFO0;
				can_filter.filter_enable = ENABLE;
				can_filter_init(&can_filter);
				can_filter.filter_number = 1;

				/* initialize filter */    
				can_filter.filter_mode = CAN_FILTERMODE_LIST;
				can_filter.filter_bits = CAN_FILTERBITS_16BIT;
				can_filter.filter_list_high = ( 0x600 | vGetNodeId() )<<5;
				can_filter.filter_list_low =  ( 0x500 | vGetNodeId() )<<5;
				can_filter.filter_mask_high = 0;
				can_filter.filter_mask_low = 0; 
				can_filter.filter_fifo_number = CAN_FIFO1;
				can_filter.filter_enable = ENABLE;
				can_filter_init(&can_filter);
				
			  can_interrupt_enable( CAN0 , CAN_INT_TME | CAN_INT_RFNE0 | CAN_INT_RFNE1   | CAN_INT_BO  );


    return ( CO_ERROR_NO );
	
	
    
}


/******************************************************************************/
void CO_CANmodule_disable(CO_CANmodule_t *CANmodule) {
	
	
	

	
	
	if (CANmodule != NULL && CANmodule->CANptr != NULL) {
		    	CAN_HandleTypeDef * hcan = CANmodule->CANptr;
		      can_working_mode_set(CAN0, CAN_MODE_INITIALIZE);
	    }
}


/******************************************************************************/
CO_ReturnError_t CO_CANrxBufferInit(
        CO_CANmodule_t         *CANmodule,
        uint16_t                index,
        uint16_t                ident,
        uint16_t                mask,
        bool_t                  rtr,
        void                   *object,
        void                  (*CANrx_callback)(void *object, void *message))
{
    CO_ReturnError_t ret = CO_ERROR_NO;

    if((CANmodule!=NULL) && (object!=NULL) && (CANrx_callback!=NULL) && (index < CANmodule->rxSize)){
        /* buffer, which will be configured */
        CO_CANrx_t *buffer = &CANmodule->rxArray[index];

        /* Configure object variables */
        buffer->object = object;
        buffer->CANrx_callback = CANrx_callback;

        /* CAN identifier and CAN mask, bit aligned with CAN module. Different on different microcontrollers. */
        buffer->ident = ident & 0x07FFU;
        if(rtr){
            buffer->ident |= 0x0800U;
        }
        buffer->mask = (mask & 0x07FFU) | 0x0800U;

        /* Set CAN hardware module filter and mask. */
        if(CANmodule->useCANrxFilters){

        }
    }
    else{
        ret = CO_ERROR_ILLEGAL_ARGUMENT;
    }

    return ret;
}


/******************************************************************************/
CO_CANtx_t *CO_CANtxBufferInit(
        CO_CANmodule_t         *CANmodule,
        uint16_t                index,
        uint16_t                ident,
        bool_t                  rtr,
        uint8_t                 noOfBytes,
        bool_t                  syncFlag)
{
    CO_CANtx_t *buffer = NULL;

    if((CANmodule != NULL) && (index < CANmodule->txSize)){
        /* get specific buffer */
        buffer = &CANmodule->txArray[index];

        /* CAN identifier, DLC and rtr, bit aligned with CAN module transmit buffer.
         * Microcontroller specific. */
        buffer->ident = ((uint32_t)ident & CANID_MASK)
                               | ((uint32_t)(rtr ? FLAG_RTR : 0x00));
        buffer->DLC = noOfBytes;
        buffer->bufferFull = false;
        buffer->syncFlag = syncFlag;
    }

    return buffer;
}






uint16_t CAN_GetTxMailboxesFreeLevel(uint32_t can_periph)
{
  uint16_t freelevel = 0U;
    /* Check Tx Mailbox 0 status */
		if ( can_flag_get(can_periph,CAN_FLAG_TME0) == SET )
   
    {
      freelevel++;
    }
    /* Check Tx Mailbox 1 status */
    if ( can_flag_get(can_periph,CAN_FLAG_TME1) == SET )
    {
      freelevel++;
    }
    /* Check Tx Mailbox 2 status */
   if ( can_flag_get(can_periph,CAN_FLAG_TME2) == SET )
    {
      freelevel++;
    }
  return ( freelevel );
}


/**
 * \brief           Send CAN message to network
 * This function must be called with atomic access.
 *
 * \param[in]       CANmodule: CAN module instance
 * \param[in]       buffer: Pointer to buffer to transmit
 */

static uint32_t prv_send_can_message(CO_CANmodule_t* CANmodule, CO_CANtx_t *buffer)
{
		can_trasnmit_message_struct transmit_message;
		uint8_t error_code;
		can_struct_para_init(CAN_TX_MESSAGE_STRUCT, &transmit_message);
		transmit_message.tx_dlen = buffer->DLC;
		for (int i=0;i<buffer->DLC;i++)
		{
			transmit_message.tx_data[i] = buffer->data[i];
		}
		transmit_message.tx_sfid = buffer->ident & CANID_MASK;
		transmit_message.tx_ff =  (buffer->ident & FLAG_RTR) ? CAN_FT_REMOTE  : CAN_FT_DATA;
		CAN_HandleTypeDef * can = CANmodule->CANptr;
		error_code =  can_message_transmit( CAN0, &transmit_message);
    return ((error_code != CAN_NOMAILBOX) ? 1 : 0U);
}



/******************************************************************************/
CO_ReturnError_t CO_CANsend(CO_CANmodule_t *CANmodule, CO_CANtx_t *buffer){
    CO_ReturnError_t err = CO_ERROR_NO;

    /* Verify overflow */
    if(buffer->bufferFull){
        if(!CANmodule->firstCANtxMessage){
            /* don't set error, if bootup message is still on buffers */
            CANmodule->CANerrorStatus |= CO_CAN_ERRTX_OVERFLOW;
        }
        err = CO_ERROR_TX_OVERFLOW;
    }

    CO_LOCK_CAN_SEND(CANmodule);
    /* if CAN TX buffer is free, copy message to it */
    if (prv_send_can_message(CANmodule, buffer) ) {
          CANmodule->bufferInhibitFlag = buffer->syncFlag;
      } else {
          buffer->bufferFull = true;
          CANmodule->CANtxCount++;
      }
    CO_UNLOCK_CAN_SEND(CANmodule);
    return err;
}


/******************************************************************************/
void CO_CANclearPendingSyncPDOs(CO_CANmodule_t *CANmodule){
    uint32_t tpdoDeleted = 0U;

    CO_LOCK_CAN_SEND(CANmodule);
    /* Abort message from CAN module, if there is synchronous TPDO.
     * Take special care with this functionality. */
    if(/*messageIsOnCanBuffer && */CANmodule->bufferInhibitFlag){
        /* clear TXREQ */
        CANmodule->bufferInhibitFlag = false;
        tpdoDeleted = 1U;
    }
    /* delete also pending synchronous TPDOs in TX buffers */
    if(CANmodule->CANtxCount != 0U){
        uint16_t i;
        CO_CANtx_t *buffer = &CANmodule->txArray[0];
        for(i = CANmodule->txSize; i > 0U; i--){
            if(buffer->bufferFull){
                if(buffer->syncFlag){
                    buffer->bufferFull = false;
                    CANmodule->CANtxCount--;
                    tpdoDeleted = 2U;
                }
            }
            buffer++;
        }
    }
    CO_UNLOCK_CAN_SEND(CANmodule);


    if(tpdoDeleted != 0U){
        CANmodule->CANerrorStatus |= CO_CAN_ERRTX_PDO_LATE;
    }
}


/******************************************************************************/
/* Get error counters from the module. If necessary, function may use
    * different way to determine errors. */
static uint16_t rxErrors=0, txErrors=0, overflow=0;

void CO_CANmodule_process(CO_CANmodule_t *CANmodule) {
    uint32_t err;

    err = ((uint32_t)txErrors << 16) | ((uint32_t)rxErrors << 8) | overflow;

    if (CANmodule->errOld != err) {
        uint16_t status = CANmodule->CANerrorStatus;

        CANmodule->errOld = err;

        if (txErrors >= 256U) {
            /* bus off */
            status |= CO_CAN_ERRTX_BUS_OFF;
        }
        else {
            /* recalculate CANerrorStatus, first clear some flags */
            status &= 0xFFFF ^ (CO_CAN_ERRTX_BUS_OFF |
                                CO_CAN_ERRRX_WARNING | CO_CAN_ERRRX_PASSIVE |
                                CO_CAN_ERRTX_WARNING | CO_CAN_ERRTX_PASSIVE);

            /* rx bus warning or passive */
            if (rxErrors >= 128) {
                status |= CO_CAN_ERRRX_WARNING | CO_CAN_ERRRX_PASSIVE;
            } else if (rxErrors >= 96) {
                status |= CO_CAN_ERRRX_WARNING;
            }

            /* tx bus warning or passive */
            if (txErrors >= 128) {
                status |= CO_CAN_ERRTX_WARNING | CO_CAN_ERRTX_PASSIVE;
            } else if (rxErrors >= 96) {
                status |= CO_CAN_ERRTX_WARNING;
            }

            /* if not tx passive clear also overflow */
            if ((status & CO_CAN_ERRTX_PASSIVE) == 0) {
                status &= 0xFFFF ^ CO_CAN_ERRTX_OVERFLOW;
            }
        }

        if (overflow != 0) {
            /* CAN RX bus overflow */
            status |= CO_CAN_ERRRX_OVERFLOW;
        }

        CANmodule->CANerrorStatus = status;
    }
}


/******************************************************************************/




 void CAN_SendMessage()
{
	CANModule_local->firstCANtxMessage = false;
	CANModule_local->bufferInhibitFlag = false;
    if(CANModule_local->CANtxCount > 0U)
    {
        uint16_t i;             /* index of transmitting message */
        CO_CANtx_t *buffer = &CANModule_local->txArray[0];
        CO_LOCK_CAN_SEND(CANModule_local);

        for (i = CANModule_local->txSize; i > 0U; --i, ++buffer) {
                   /* Try to send message */
                   if (buffer->bufferFull) {
                       if (prv_send_can_message(CANModule_local, buffer)) {
                           buffer->bufferFull = false;
                           CANModule_local->CANtxCount--;
                           CANModule_local->bufferInhibitFlag = buffer->syncFlag;
                       }
                   }
               }
        /* Clear counter if no more messages */
        if(i == 0U){
        	CANModule_local->CANtxCount = 0U;
        }
        CO_UNLOCK_CAN_SEND(CANModule_local);
    }
}

/*
 *
 */

int CAN_GetRxMessage(uint32_t can_periph, uint32_t RxFifo,  CO_CANrxMsg_t * pCANData)
{
 
     can_receive_message_struct g_receive_message;
     /* initialize receive message */
     can_struct_para_init(CAN_RX_MESSAGE_STRUCT, &g_receive_message);
     /* check the receive message */
     can_message_receive(can_periph, RxFifo, &g_receive_message);
     if (g_receive_message.rx_ff == CAN_FF_STANDARD )
     {  
    	 pCANData->ident =   g_receive_message.rx_sfid & CAN_SFID_MASK | ( ( g_receive_message.rx_ft == CAN_FT_REMOTE) ? FLAG_RTR : 0x00 );
    	 pCANData->dlc = g_receive_message.rx_dlen;
    	 pCANData->data[0] = (uint8_t) g_receive_message.rx_data[0];
			 pCANData->data[1] = (uint8_t) g_receive_message.rx_data[1];
    	 pCANData->data[2] = (uint8_t) g_receive_message.rx_data[2];
    	 pCANData->data[3] = (uint8_t) g_receive_message.rx_data[3];
			 pCANData->data[4] = (uint8_t) g_receive_message.rx_data[4];
			 pCANData->data[5] = (uint8_t) g_receive_message.rx_data[5];
    	 pCANData->data[6] = (uint8_t) g_receive_message.rx_data[6];
    	 pCANData->data[7] = (uint8_t) g_receive_message.rx_data[7];
    }
    return 1;
}

/*
Фунеуия колбек для отправики нового сообщения из очереди
*/
 void  prv_read_can_received_msg(uint32_t can_periph, uint32_t fifo) 
{

    static CO_CANrxMsg_t rcvMsg;
    if (CAN_GetRxMessage( CAN0 , fifo,  &rcvMsg) == 1)
    {
    	CO_CANrx_t * buffer = CANModule_local->rxArray;
    	    for (uint8_t index = CANModule_local->rxSize; index > 0U; --index, ++buffer)
    	    {
    	         if (((rcvMsg.ident ^ buffer->ident) & buffer->mask) == 0U)
    	         {
    	              if (buffer != NULL && buffer->CANrx_callback != NULL)
    	              {
    	                  buffer->CANrx_callback(buffer->object, (void*) &rcvMsg);
    	              }
    	              break;
    	          }
    	    }
    }
    return;
}

/*
Callback для прерывания ошибки CAN BusOff
*/
void HAL_CAN_ErrorCallback( uint32_t can_periph )
{
	if ( can_interrupt_flag_get(CAN0, CAN_INT_FLAG_BOERR ) == SET ) 
	{
		vRestartNode();
	}
}

