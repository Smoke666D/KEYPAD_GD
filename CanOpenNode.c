/*
 * CanOpenNode.c
 *
 *  Created on: Nov 17, 2021
 *      Author: igor.dymov
 */

#include "CANOpen.h"
#include "CanOpenNode.h"
#include "CO_driver_ST32F103.h"
#include "OD.h"
#include "FreeRTOS.h"
#include "task.h"
/*
 * Локальные переменные
 */
static CO_t* CO;
static uint32_t co_heap_used;
static CO_NMT_reset_cmd_t reset = CO_RESET_NOT;
static CAN_HandleTypeDef *pcan;

/* Default values for CO_CANopenInit() */
#define NMT_CONTROL                     ( vFDGetNMTState() | CO_NMT_ERR_ON_ERR_REG | CO_ERR_REG_GENERIC_ERR | CO_ERR_REG_COMMUNICATION)
#define FIRST_HB_TIME                   500
#define SDO_SRV_TIMEOUT_TIME            1000
#define SDO_CLI_TIMEOUT_TIME            500
#define SDO_CLI_BLOCK                   false
#define OD_STATUS_BITS                  NULL


void vCanOpenInit(CAN_HandleTypeDef *hcan)
{
	pcan = hcan;
	return;
}

void vCanOpenProcess(void *argument)
{
	uint32_t max_sleep_time_us;
   /*Инициализация нового объекта CANOpen*/
	if ((CO = CO_new(NULL, &co_heap_used)) == NULL)
	{
	      Error_Handler();
	}
	CO->CANmodule->CANptr = pcan;

	/*Создание объектов синхронизации FreeRTOS*/
	co_drv_create_os_objects();
	/*Получение доступа к mutex перед тек как запусить переодический процесс*/
	co_drv_mutex_lock();

	/*Старатуем приложение*/
	do {
		uint16_t pendingBitRate = vGetBitrate();
		uint32_t errInfo = 0, time_old, time_current;
		uint8_t pendingNodeId = vGetNodeId(), activeNodeId = 0;
		CO_ReturnError_t err;

		/* Reset normal state */
		 CO->CANmodule->CANnormal = false;
		 /* Enter CAN configuration. May be NULL, default one is used in driver */
		 CO_CANsetConfigurationMode(CO->CANmodule->CANptr);
		 CO_CANmodule_disable(CO->CANmodule);

		 /* Initialize CANopen */
		  if ((err = CO_CANinit(CO, CO->CANmodule->CANptr, pendingBitRate)) != CO_ERROR_NO)
		  {
		      Error_Handler();
		  }
		  CO_LSS_address_t lssAddress = {
	                .identity = {
	                .vendorID       = OD_ROM.x1018_identity.vendor_ID,
	                .productCode    = OD_ROM.x1018_identity.productCode,
	                .revisionNumber = OD_ROM.x1018_identity.revisionNumber,
	                .serialNumber   = OD_ROM.x1018_identity.serialNumber
	            }
	       };
	       if ((err = CO_LSSinit(CO, &lssAddress, &pendingNodeId, &pendingBitRate)) != CO_ERROR_NO)
	       {
	            Error_Handler();
	       }
	        /* Initialite core stack */
	        activeNodeId = pendingNodeId;
	        err = CO_CANopenInit(CO,                /* CANopen object */
	                             NULL,              /* alternate NMT */
	                             NULL,              /* alternate em */
	                             OD,                /* Object dictionary */
	                             OD_STATUS_BITS,    /* Optional OD_statusBits */
	                             NMT_CONTROL,       /* CO_NMT_control_t */
	                             FIRST_HB_TIME,     /* firstHBTime_ms */
	                             SDO_SRV_TIMEOUT_TIME, /* SDOserverTimeoutTime_ms */
	                             SDO_CLI_TIMEOUT_TIME, /* SDOclientTimeoutTime_ms */
	                             SDO_CLI_BLOCK,     /* SDOclientBlockTransfer */
	                             activeNodeId,
	                             &errInfo);
	        if (err != CO_ERROR_NO && err != CO_ERROR_NODE_ID_UNCONFIGURED_LSS)
	        {
	        	 Error_Handler();
	        }

	        /* Initialize PDO */
	        err = CO_CANopenInitPDO(CO, CO->em, OD, activeNodeId, &errInfo);
	        if (err != CO_ERROR_NO) {

	            Error_Handler();
	        }

	        /* Configure CANopen callbacks, etc */
	        if (!CO->nodeIdUnconfigured) {
	#if (CO_CONFIG_STORAGE) & CO_CONFIG_STORAGE_ENABLE
	            if (storageInitError != 0) {
	                CO_errorReport(CO->em, CO_EM_NON_VOLATILE_MEMORY,
	                               CO_EMC_HARDWARE, storageInitError);
	            }
	#endif
	        } else {

	            Error_Handler();
	        }

	        /* Start CAN to receive messages */
	        CO_CANsetNormalMode(CO->CANmodule);

	        reset = CO_RESET_NOT;


	        /* Release semaphore at this point. We are ready to proceed */
	        co_drv_mutex_unlock();

	        /* Get current tick time */
	        time_old = time_current = osKernelGetTickCount();
	        max_sleep_time_us = 0;              /* Get first sleep time */
	        while (reset == CO_RESET_NOT) {
	            uint32_t timeDifference_us;

	            /*
	             * This will block this thread for up to maximal time.
	             *
	             * If new CAN message arrives,
	             * thread will be woken-up from CAN RX interrupt,
	             * and processing will continue immediately
	             */
	            CO_WAIT_SYNC_APP_THREAD(max_sleep_time_us / 1000);

	            /* Get exclusive access to CANopen core stack. */
	            co_drv_mutex_lock();

	            /* Set time to max sleep in next iteration */
	            max_sleep_time_us = 10000;

	            /* Get current kernel tick time */
	            time_current = xTaskGetTickCount();
	            timeDifference_us = (time_current - time_old) * (1000000 / configTICK_RATE_HZ);
	            time_old = time_current;

	            /* CANopen process */
	            reset = CO_process(CO, false, timeDifference_us, &max_sleep_time_us);
	            /*
	             * We want to wakeup periodic thread,
	             * in-charge of other important CANopen tasks
	             */
	            CO_WAKEUP_PERIODIC_THREAD();

	            /* Only now release mutex to allow other tasks accessing CANopen core */
	            co_drv_mutex_unlock();
	        }

   } while (reset == CO_RESET_NOT);


	 /* Final end loop that never ends */
	    while (1) {
	        NVIC_SystemReset();
	    }
}
void vCanOpenPeriodicProcess(void *argument)
{
	uint32_t time_old, time_current, timeDifference_us, max_sleep_time_us;
	co_drv_mutex_lock();
	time_old = time_current = xTaskGetTickCount();
	max_sleep_time_us = 0;
	for(;;)
	{
		co_drv_mutex_unlock();                  /* Release mutex to allow other tasks to process */
		CO_WAIT_SYNC_PERIODIC_THREAD(max_sleep_time_us / 1000);

		/* Get access mutex */
	    co_drv_mutex_lock();

		/* Set time to maximum wait in next loop */
		max_sleep_time_us = 10000;

		/* Verify that everything is set */
		if (CO == NULL || !CO->CANmodule->CANnormal) {
		        continue;
		}
		time_current = xTaskGetTickCount();
		timeDifference_us = (time_current - time_old) * (1000000 / configTICK_RATE_HZ);
		time_old = time_current;

		/* For the moment lock interrupts for further processing */
	   if (!CO->nodeIdUnconfigured && CO->CANmodule->CANnormal) {
		            bool_t syncWas = false;

		#if (CO_CONFIG_SYNC) & CO_CONFIG_SYNC_ENABLE
		            syncWas = CO_process_SYNC(CO, timeDifference_us, &max_sleep_time_us);
		#endif
		#if (CO_CONFIG_PDO) & CO_CONFIG_RPDO_ENABLE
		            CO_process_RPDO(CO, syncWas, timeDifference_us, &max_sleep_time_us);
		#endif
		#if (CO_CONFIG_PDO) & CO_CONFIG_TPDO_ENABLE
		            CO_process_TPDO(CO, syncWas, timeDifference_us, &max_sleep_time_us);
		#endif
		        }

	}

}
