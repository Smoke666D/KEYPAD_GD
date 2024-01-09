/*
 * led.c
 *
 *  Created on: Nov 12, 2021
 *      Author: igor.dymov
 */
#include "led.h"
#include "math.h"
#include "init.h"

static uint8_t LED_ON[SPI_PACKET_SIZE] 			=         { 0x00 , 0x00 , 0x00 };
static uint8_t LED_BLINK[SPI_PACKET_SIZE]    	=         { 0x00 , 0x00 , 0x00 };

static uint16_t backligch_brigth		 = 0x1F;
static uint16_t led_brigth 				 = 0x3F;
static uint8_t color_div 				 = 1U;
static uint32_t pwmtim		 = NULL;
static uint8_t brigth_color[SPI_PACKET_SIZE];
static uint16_t led_brigth_counter 		= 0;
static uint16_t led_blink_counter 		= 0;
static uint8_t BlinkON					= 1;
static uint8_t ucSPI_Busy 				= 0;
/*
 * Защелка данных в SPI буферах
 * Поскольку длителность импульса мала, не целесобразно делать задержку через что-то, кроме пустого цикла
 */
static void vLatch( void )
{
	gpio_bit_set(CS_GPIO_Port, CS_Pin);
	for (uint8_t i = 0U; i < 200; i++);
	gpio_bit_reset(CS_GPIO_Port, CS_Pin);
	return;
}
/*
 *
 */
static void vSPI_Transmit( uint8_t *pData)
{
  uint32_t tickstart = timer_counter_read(TIMER0); //HAL_GetTick();
  if ( ! ucSPI_Busy )
  {
	  ucSPI_Busy = 1;
		spi_enable(SPI1);
		spi_i2s_data_frame_format_config(SPI1, SPI_FRAMESIZE_8BIT);
	  for (uint8_t i = 0; i< SPI_PACKET_SIZE; i++)
	  {
    	while ( spi_i2s_flag_get(SPI1, SPI_FLAG_TBE) == RESET)
    	{
    		if ( (timer_counter_read(TIMER0) - tickstart) >=  SPI_TIMEOUT)
    		{
    			goto error;
    		}
    	}
    	spi_i2s_data_transmit(SPI1,pData[i]);
	  }
	   while ( spi_i2s_flag_get(SPI1,SPI_FLAG_TRANS)  == SET)     
	   {
			if ( (timer_counter_read(TIMER0) - tickstart) >=  SPI_TIMEOUT)
	        {
						
	         /* Disable TXE, RXNE and ERR interrupts for the interrupt process */
						spi_i2s_interrupt_disable(SPI1,SPI_I2S_INT_TBE);
						spi_i2s_interrupt_disable(SPI1,SPI_I2S_INT_RBNE);
						spi_i2s_interrupt_disable(SPI1,SPI_I2S_INT_ERR);
	    	   goto error;
	       }
	   }
	   //LL_SPI_ClearFlag_OVR(SPI2);
  }
  vLatch();
 error:
  ucSPI_Busy = 0;
  return;
}

/*
 *
 */
static uint16_t calcBrigt(uint8_t pbr)
{
  return ( ( pbr > MAX_BRIGTH )?  MAX_BRIGTH : (uint16_t) ( sin((double)pbr*(3.14/2.0)/MAX_BRIGTH)*(MAX_BRIGTH_COUNTER) ) );
}
/*
 *
 */
static uint8_t vSTPErrorDetection()
{
	/*Входим в режим Detectiom*/
	 gpio_bit_set(GPIOA, GPIO_PIN_0|GPIO_PIN_1); /*OE High*/
	 gpio_bit_set(GPIOB, GPIO_PIN_10);
	 gpio_bit_reset(CS_GPIO_Port, CS_Pin);  /*LE low*/
	 for (uint8_t i=0;i<(5+8*3);i++)
	 {

		 gpio_bit_set(CS_GPIO_Port, CS_Pin);
		 vTaskDelay(1);
		 gpio_bit_reset(CS_GPIO_Port, CS_Pin);
		 switch (i)
		 {
		    case 0:
		    	gpio_bit_reset(GPIOA, GPIO_PIN_0|GPIO_PIN_1); /*OE Low*/
		    	gpio_bit_reset(GPIOB, GPIO_PIN_10);
		    	break;
		    case 1:
		    	gpio_bit_set(GPIOA, GPIO_PIN_0|GPIO_PIN_1); /*OE High*/
		    	gpio_bit_set(GPIOB, GPIO_PIN_10);
		    	break;
		    case 2:
		    	gpio_bit_set(CS_GPIO_Port, CS_Pin);  /*LE High*/
		    	break;
		    case 3:
		    	gpio_bit_reset(CS_GPIO_Port, CS_Pin);  /*LE low*/
		    	break;
		    case 4:
		    	gpio_bit_set(GPIOA, GPIO_PIN_15);
		    	break;
		    default:
		    	break;
		 }
	 }
	 gpio_bit_reset(GPIOA, GPIO_PIN_15);
	 vLatch();
	 gpio_bit_reset(GPIOA, GPIO_PIN_0|GPIO_PIN_1); /*OE Low*/
	 gpio_bit_reset(GPIOB, GPIO_PIN_10);
	 vTaskDelay(1);
	 gpio_bit_set(CS_GPIO_Port, CS_Pin);
	 vTaskDelay(1);
	 gpio_bit_reset(CS_GPIO_Port, CS_Pin);
	 gpio_bit_reset(GPIOA, GPIO_PIN_0|GPIO_PIN_1); /*OE Low*/
	 gpio_bit_reset(GPIOB, GPIO_PIN_10);
	 uint32_t buf=0;
	 for (uint8_t i =0;i<(3*8);i++)
	 {
		 gpio_bit_set(CS_GPIO_Port, CS_Pin);
		 vTaskDelay(1);
		 if ( gpio_input_bit_get(GPIOA, GPIO_PIN_14) == SET )
         {
			 buf=buf<<1;
			 buf |= 0x0001;
         }
		 gpio_bit_reset(CS_GPIO_Port, CS_Pin);
		 vTaskDelay(1);
	 }
	 return ((buf == 0x0FFFFFF)? 0U : 1U );
}
/*
 *
 */
static void vSTPNormalMode()
{
	 gpio_bit_set(GPIOA, GPIO_PIN_0|GPIO_PIN_1); /*OE High*/
	 gpio_bit_set(GPIOB, GPIO_PIN_10);
	 gpio_bit_reset(CS_GPIO_Port, CS_Pin);  /*LE low*/
	 for (uint8_t i=0;i<5;i++)
	 {

	    gpio_bit_set(CS_GPIO_Port, CS_Pin);
	    vTaskDelay(1);
		  gpio_bit_reset(CS_GPIO_Port, CS_Pin);
		switch (i)
		{
		    case 1:
		    	gpio_bit_reset(GPIOA, GPIO_PIN_0|GPIO_PIN_1); /*OE Low*/
		    	gpio_bit_reset(GPIOB, GPIO_PIN_10);
		    	break;
		    case 2:
		    	gpio_bit_set(GPIOA, GPIO_PIN_0|GPIO_PIN_1); /*OE High*/
		    	gpio_bit_set(GPIOB, GPIO_PIN_10);
		    	break;
		    default:
		    	break;
		 }
	 }
	 return;
}
/*
 *
 */
void vLedInit(uint32_t htim)
{
	pwmtim = htim;
	vSetBackLigthColor(vFDGetRegState(DEF_BL_COLOR_ADR));
	vSetLedBrigth(vFDGetRegState(DEF_LED_BRIGTH_ADR));
	vSetBackLigth(vFDGetRegState(DEF_BL_BRIGTH_ADR));
  vSetBrigth(MAX_BRIGTH);

    return;
}

void vStartLed( void )
{
  // LL_TIM_EnableIT_UPDATE(TIM4);
	 //timer_update_event_enable(TIMER3);
  // LL_TIM_EnableCounter(TIM4);
   timer_enable (TIMER3);
   return;
}
/*
 *
 */
void vLedDriverStart(void)
{
	gpio_init(GPIOA,GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0|GPIO_PIN_1 |GPIO_PIN_15);
	gpio_init(GPIOB,GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
	gpio_init(GPIOA,GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
	if (vSTPErrorDetection() == 0)
	{
		vSTPNormalMode();
	}
	gpio_init(GPIOA,GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_15);
	gpio_init(GPIOB,GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
	gpio_init(GPIOA,GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
	return;
}
/*
 * Функция включения светодиодоы.
 * Корректность аргументов проверятся при вызове
 */
void vSetLedOn(uint8_t Color,uint8_t State)
{
	LED_ON[Color-1] = State;
	return;
}
/*
 *
 */
void vSetLedBlink(uint8_t Color, uint8_t State)
{
	LED_BLINK[Color-1] = State;
	return;
}

/*
 *
 */
void vSetLedBrigth(uint8_t brigth)
{
	led_brigth = calcBrigt(brigth);
	return;
}

void vSetBackLigthColor(uint8_t color)
{
	brigth_color[0]=MAX_DATA;
	brigth_color[1]=MAX_DATA;
	brigth_color[2]=MAX_DATA;
	color_div =2;
	switch (color)
	{
		case  RED:
			brigth_color[1]=0x00;
			brigth_color[2]=0x00;
			color_div =1;
			break;
		case GREEN:
			brigth_color[0]=0x00;
			brigth_color[2]=0x00;
			color_div =1;
			break;
		case BLUE:
			brigth_color[0]=0x00;
			brigth_color[1]=0x00;
			color_div =1;
			break;
		case YELLOW:
			brigth_color[2]=0x00;
			break;
		case YELLOW_GREEN:
			brigth_color[2]=0x00;
			break;
		case  AMBER:
			brigth_color[2]=0x00;
			break;
		case VIOLET:
			brigth_color[1]=0x00;
			break;
		case CYAN:
			brigth_color[0]=0x00;
			break;
		case WHITE:
		default:
			break;
	}
	return;
}

/*
 *
 */
void vSetBackLigth(uint8_t brigth)
{
	backligch_brigth =calcBrigt( brigth);
}
/*
 * Функция установки якрости.
 */
void vSetBrigth(uint8_t brigth)
{
	if (brigth <= MAX_BRIGTH)
	{
	
		/*
		 * Яркость устанвливается для каждого цвета отдельно. Возможно задавать индивидуалное соотношение яркостей цветов для получения
		 * дополнительных переходных цветов, например  AMBER и YELLOW_GREEN
		 */
		timer_channel_output_state_config(pwmtim, TIMER_CH_0, TIMER_CCX_DISABLE);
    timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_0,(uint16_t)( ( (float)(brigth)/MAX_BRIGTH )* PWM_TIM_PERIOD ) );
		timer_channel_output_state_config(pwmtim, TIMER_CH_0, TIMER_CCX_ENABLE);
		
		
		timer_channel_output_state_config(pwmtim, TIMER_CH_1, TIMER_CCX_DISABLE);
    timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_1,(uint16_t)( ( (float)(brigth)/MAX_BRIGTH )* PWM_TIM_PERIOD ) );
		timer_channel_output_state_config(pwmtim, TIMER_CH_1, TIMER_CCX_ENABLE);
		
		
		timer_channel_output_state_config(pwmtim, TIMER_CH_2, TIMER_CCX_DISABLE);
    timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_2,(uint16_t)( ( (float)(brigth)/MAX_BRIGTH )* PWM_TIM_PERIOD )  );
		timer_channel_output_state_config(pwmtim, TIMER_CH_2, TIMER_CCX_ENABLE);
		
	
	}
}
/*
 *  Функция вывода данных в SPI, вызывается по прерыванию таймра №4
 */
void vLedProcess( void )
{
	/*Cбравысваем флаг тамера 4*/
	

	uint8_t data[SPI_PACKET_SIZE];
	uint8_t temp_led;
	led_brigth_counter++;
    if (led_brigth_counter>MAX_BRIGTH_COUNTER)
    {
    	led_brigth_counter = 0;
    }
    if (LED_BLINK[0] || LED_BLINK[1] || LED_BLINK[2])
    {

        if (++led_blink_counter > 22500U)
        {
        	led_blink_counter = 0;
        	BlinkON	 =  (BlinkON ==1) ? 0 : 1;
        }
    	if (BlinkON == 1)
    	{
    		LED_ON[0] |= LED_BLINK[0];
    		LED_ON[1] |= LED_BLINK[1];
    		LED_ON[2] |= LED_BLINK[2];
    	}
    	else
    	{
    		LED_ON[0] &= ~LED_BLINK[0];
    		LED_ON[1] &= ~LED_BLINK[1];
    		LED_ON[2] &= ~LED_BLINK[2];
    	}
    }
    else
    {
    	BlinkON = 1U;
    }
    data[0] =0;
    data[1] =0;
    data[2] =0;
    temp_led = ~(LED_ON[0]  | LED_ON[1] | LED_ON[2] );
	if (led_brigth_counter < backligch_brigth/color_div)
	{
		 data[2]=brigth_color[0] & temp_led;
	 	 data[1]=brigth_color[1] & temp_led;
	 	 data[0]=brigth_color[2] & temp_led;
 	}
	if (led_brigth_counter <= led_brigth)
	{
		data[2]|=LED_ON[0];
	 	data[1]|=LED_ON[1];
	 	data[0]|=LED_ON[2];
	 }
	 vSPI_Transmit(&data[0]);
   return;
}

