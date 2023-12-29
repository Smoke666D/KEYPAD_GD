/*
 * flash_data.c
 *
 *  Created on: Dec 1, 2021
 *      Author: igor.dymov
 */

#include "flash_data.h"
#include "CO_NMT_Heartbeat.h"
#include "led.h"
#include "CANopen.h"
#include "CO_ODinterface.h"
#include "OD.h"



static uint8_t FisrtStart = 1; // 0         1    2     3     4     5      6     7  8  9  10
static uint8_t SettingsREG[REG_SIZE]={VALID_CODE, 0x02, 0x15, 0x01, 0x3F, 0x00, WHITE, 1 ,2, 2, 4, 3, 00, 44,00,55};

static uint8_t *MEM_If_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
static void MEM_If_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len);

;
static void vFDWtiteReg(void);

void * cgetREGAdr(uint8_t adr)
{
	return ((void *)&SettingsREG[adr]);
}

static void vFDWtiteReg(void)
{
	uint8_t * src = (uint8_t *) FLASH_DATA_ADR;
	fmc_unlock( );
	fmc_page_erase(FLASH_DATA_ADR);
	MEM_If_Write_FS(&SettingsREG[0], src, sizeof(SettingsREG));
	fmc_lock( );
}

void vFDInit( void )
{
	uint8_t * src =  (uint8_t *) FLASH_DATA_ADR;
	uint8_t  buff;
	if (FisrtStart)
	{
	   MEM_If_Read_FS(src, &buff, 1);
	   if (buff!= VALID_CODE)
	   {
		   vFDWtiteReg();
	   }
		MEM_If_Read_FS(src, &SettingsREG[0],  sizeof(SettingsREG));
		FisrtStart = 0;
		OD_set_value(OD_ENTRY_H2004_keyBoardParametr,0x01,&SettingsREG[KEYBOARD_PERIOD_ADRRES ], 1, true);
		OD_set_value(OD_ENTRY_H2004_keyBoardParametr,0x02,&SettingsREG[KEYBOARD_PERIOD_ADRRES +1 ], 1, true);
		OD_set_value(OD_ENTRY_H2004_keyBoardParametr,0x03,&SettingsREG[KEYBOARD_PERIOD_ADRRES +2 ], 1, true);
		OD_set_value(OD_ENTRY_H2004_keyBoardParametr,0x04,&SettingsREG[KEYBOARD_PERIOD_ADRRES +3 ], 1, true);
		OD_set_value(OD_ENTRY_H2005_PWM_Parametr,0x01,&SettingsREG[KEYBOARD_PERIOD_ADRRES +3 ], 1, true);
		OD_set_value(OD_ENTRY_H2005_PWM_Parametr,0x02,&SettingsREG[KEYBOARD_PERIOD_ADRRES +3 ], 1, true);
	}
}
/*
 *
 */
void vFDSetRegState(uint8_t adr, uint8_t state)
{
	SettingsREG[adr]= state;
	vFDWtiteReg();
}


/*
 *
 */
void vFDSetRegState16(uint8_t adr, uint16_t state)
{
	memmove(&SettingsREG[adr], &state, sizeof(state));
	vFDWtiteReg();
}


uint8_t vFDGetRegState(uint8_t adr)
{
	if (FisrtStart)
	{
	  vFDInit();
	}
	return SettingsREG[adr];
}

uint16_t vGetBitrate()
{
	uint16_t data = 0;
	 if (FisrtStart)
	 {
		  vFDInit();
	 }
	switch(SettingsREG[BITRATE_ADR])
	{
		case 0x00:
			data = 1000;
			break;
		case 0x02:
			data = 500;
			break;
		case 0x03:
			data = 250;
			break;
		case 0x04:
			data = 125;
			break;
		case 0x06:
			data =50;
			break;
		case 0x07:
			data = 20;
			break;
		default:
			data = 125;
			break;
	}
    return data;
}

uint16_t vFDGetNMTState( void )
{
	 uint16_t res = 0;
	 if (FisrtStart) {
	   vFDInit();
     }
	 if (SettingsREG[NMT_STATE_ADR] == 0x01) {
		 res = CO_NMT_STARTUP_TO_OPERATIONAL;
	 }
	 return res;
}

uint8_t vFDGetStartMessage( void )
{
	if (FisrtStart)
	  {
		  vFDInit();
	  }
	  return SettingsREG[NMT_START_MESSAGE];

}

uint8_t vGetNodeId( void )
{
  if (FisrtStart)
  {
	  vFDInit();
  }
  return SettingsREG[NODE_ID_ADR];
}







/**
  * @brief  Memory write routine.
  * @param  src: Pointer to the source buffer. Address to be written to.
  * @param  dest: Pointer to the destination buffer.
  * @param  Len: Number of data to be written (in bytes).
  * @retval USBD_OK if operation is successful, MAL_FAIL else.
  */
void MEM_If_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len)
{
  /* USER CODE BEGIN 3 */
  uint32_t           i      = 0U; 

  for ( i=0U; i<Len; i+=4U )
  {
	if ( ( uint32_t )( dest + i ) > FLASH_SIZE )
	{
      if (   fmc_word_program( ( uint32_t )( dest + i ), *( uint32_t* )( src + i ) ) == FMC_READY)
      {
        if ( *( uint32_t* )( src + i ) != *( uint32_t* )( dest + i ) )
        {
        	
          break;
        }
        else
        {
        	
        }
      }
      else
      {
    	  
        break;
      }
	}
  }
  return;
  /* USER CODE END 3 */
}

/**
  * @brief  Memory read routine.
  * @param  src: Pointer to the source buffer. Address to be written to.
  * @param  dest: Pointer to the destination buffer.
  * @param  Len: Number of data to be read (in bytes).
  * @retval Pointer to the physical address where data should be read.
  */
uint8_t *MEM_If_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len)
{
  /* Return a valid address to avoid HardFault */
  /* USER CODE BEGIN 4 */

    uint32_t i    = 0U;
    uint8_t *psrc = src;

    for ( i=0U; i<Len; i++ )
    {
      dest[i] = *psrc++;
    }
    return ( uint8_t* )( dest );

  /* USER CODE END 4 */
}



