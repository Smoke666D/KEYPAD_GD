#include "init.h"
#include "main.h"

static void MX_GPIO_Init( void );
static void MX_IWDG_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM3_Init(void);
static void MX_RCU_Init( void );
static void nvic_config(void);
static void InitBaseTick( void );
static void MX_TIM1_Init(void);
/*
Функция инициализации перефирии устройства
*/
void vInit_DeviceConfig( void )
{	
	MX_RCU_Init( );
	SystickConfig();
	MX_GPIO_Init( );
	InitBaseTick();
	MX_SPI2_Init( );
	MX_TIM1_Init();
	MX_TIM3_Init( );
	
 
	MX_CAN_Init(CAN0,500);
	
	nvic_config();
	return;
}




static void nvic_config(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
	  nvic_irq_enable(TIMER3_IRQn, 6, 0);
	  nvic_irq_enable(TIMER0_UP_IRQn, 15, 0);
	  nvic_irq_enable(USBD_HP_CAN0_TX_IRQn, 6, 0);
    nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn, 6, 0);
    nvic_irq_enable(CAN0_RX1_IRQn, 6, 0);
    nvic_irq_enable(CAN0_EWMC_IRQn , 6, 0);
}
/*
*/


static void InitBaseTick( void )
{
 
  uint32_t              uwTimclock = 0U;
  uint32_t              uwPrescalerValue = 0U;
  uint32_t              pFLatency;

  /* Enable TIM1 clock */
  timer_parameter_struct timer_initpara;
	
	 /* Compute TIM1 clock */
  uwTimclock = rcu_clock_freq_get(CK_SYS);

  /* Compute the prescaler value to have TIM1 counter clock equal to 1MHz */
 uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);
	
 timer_deinit(TIMER0);
 timer_internal_clock_config(TIMER0);
 timer_struct_para_init(&timer_initpara);
 /* TIMER1 configuration */
 timer_initpara.prescaler         = uwPrescalerValue;
 timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
 timer_initpara.counterdirection  = TIMER_COUNTER_UP;
 timer_initpara.period            = (1000000U / 1000U) - 1U;;
 timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
 timer_init(TIMER0, &timer_initpara);

 //timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_UP);
 //timer_interrupt_enable(TIMER0, TIMER_INT_UP);
 timer_enable(TIMER0);
}


static void MX_RCU_Init( void )
{
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOD);
	rcu_periph_clock_enable(RCU_AF);
  rcu_periph_clock_enable(RCU_SPI1);
	rcu_periph_clock_enable(RCU_TIMER0);
	rcu_periph_clock_enable(RCU_TIMER2);
	rcu_periph_clock_enable(RCU_TIMER1);
	rcu_periph_clock_enable(RCU_CAN0);
	return;
}

 void MX_CAN_Init( uint32_t can_periph, uint16_t bit_rate)
{
	
  /* USER CODE BEGIN CAN_Init 0 */
  can_parameter_struct can_parameter;
	can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
	can_deinit(CAN0);
	
  can_parameter.time_triggered = DISABLE;
  can_parameter.auto_bus_off_recovery = ENABLE;
  can_parameter.auto_wake_up = DISABLE;
  can_parameter.auto_retrans = DISABLE;
  can_parameter.rec_fifo_overwrite = DISABLE;
  can_parameter.trans_fifo_order = DISABLE;
  can_parameter.working_mode = CAN_NORMAL_MODE;
  can_parameter.resync_jump_width = CAN_BT_SJW_4TQ;
  can_parameter.time_segment_1 = CAN_BT_BS1_12TQ;
  can_parameter.time_segment_2 = CAN_BT_BS2_5TQ;
     
	
	 switch (bit_rate)
   {
            case 1000:  can_parameter.prescaler = 2;
                   break;
            case 500: can_parameter.prescaler = 4;
                   break;
            case 250:  can_parameter.prescaler = 8;
                   break;
            default:
            case 125:  can_parameter.prescaler = 16;
                  break;
            case 100: can_parameter.prescaler = 20;
                 break;
            case 50: can_parameter.prescaler = 120;
                 break;
            case 20:  can_parameter.prescaler= 300;
                 break;
            case 10:  can_parameter.prescaler = 600;
                 break;
    }
	
	can_init(CAN0, &can_parameter);

//Инициализация портов CAN0
	 gpio_init(GPIOB,GPIO_MODE_IPU,GPIO_OSPEED_50MHZ,GPIO_PIN_8);
   gpio_init(GPIOB,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_9);
 
  gpio_pin_remap_config(GPIO_CAN_PARTIAL_REMAP ,ENABLE);

}

/*
Функция инициализации портов
*/
static void MX_GPIO_Init(void)
{
  /* GPIO Ports Clock Enable */

  /*Configure GPIO pin Output Level */
  gpio_bit_reset(CS_GPIO_Port, CS_Pin);

  /*Configure GPIO pin Output Level */
  gpio_bit_reset(GPIOA, GPIO_PIN_8);
	
  /*Configure GPIO pins : PC13 PC14 PC15 */
	gpio_init(GPIOC,GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);
	
  /*Configure GPIO pins : PA2 PA3 PA4 PA6
                           PA7 PA9 PA10 PA11
                           PA12 PA15 */
	gpio_init(GPIOA,GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_15);
	
  /*Configure GPIO pin : KL1_Pin */
  gpio_init(KL1_GPIO_Port,GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, KL1_Pin);

  /*Configure GPIO pins : PB0 PB11 */
	gpio_init(GPIOB,GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0|GPIO_PIN_11);

  /*Configure GPIO pins : KL2_Pin KL3_Pin PB14 KL4_Pin
                           KL5_Pin KL6_Pin KL7_Pin KL8_Pin */
  gpio_init(GPIOB,GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, KL2_Pin|KL3_Pin|GPIO_PIN_14|KL4_Pin
                          |KL5_Pin|KL6_Pin|KL7_Pin|KL8_Pin);

  /*Configure GPIO pin : CS_Pin */
  gpio_init(CS_GPIO_Port,GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, CS_Pin);

  /*Configure GPIO pin : PA8 */
  gpio_init(GPIOA,GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_8);
	
	gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 | GPIO_PIN_15);

  
}




/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
	
	fwdgt_config(4095, FWDGT_PSC_DIV64);
	fwdgt_enable();

 
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{
  spi_parameter_struct spi_init_struct;
  spi_i2s_deinit(SPI1);
  
  /**SPI2 GPIO Configuration
  PB13   ------> SPI2_SCK
  PB15   ------> SPI2_MOSI
  */
  
	
	spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
	spi_init_struct.device_mode          = SPI_MASTER;
	spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
	spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
	spi_init_struct.nss                  = SPI_NSS_SOFT;
	spi_init_struct.prescale             = SPI_PSC_8;
	spi_init_struct.endian               = SPI_ENDIAN_MSB;
	spi_init(SPI1, &spi_init_struct);
	
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

	
	timer_oc_parameter_struct timer_ocintpara;
  timer_parameter_struct timer_initpara;
 
  timer_deinit(TIMER1);
	
	timer_internal_clock_config(TIMER1);
	
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
  gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

gpio_pin_remap_config(GPIO_TIMER1_PARTIAL_REMAP1,ENABLE);
	timer_struct_para_init(&timer_initpara);
	timer_initpara.prescaler         = 2;
  timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
  timer_initpara.counterdirection  = TIMER_COUNTER_UP;
  timer_initpara.period            = PWM_TIM_PERIOD;
  timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
  timer_initpara.repetitioncounter = 0;
  timer_init(TIMER1,&timer_initpara);
	
	timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
	timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
	timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
	timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
	timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
	timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
  /* USER CODE END TIM2_Init 1 */

	timer_channel_output_config(TIMER1,TIMER_CH_0,&timer_ocintpara);
	timer_channel_output_config(TIMER1,TIMER_CH_1,&timer_ocintpara);
	timer_channel_output_config(TIMER1,TIMER_CH_2,&timer_ocintpara);
	
	timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_0,500);
  timer_channel_output_mode_config(TIMER1,TIMER_CH_0,TIMER_OC_MODE_PWM1);
  timer_channel_output_shadow_config(TIMER1,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);
	
	timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_1,500);
  timer_channel_output_mode_config(TIMER1,TIMER_CH_1,TIMER_OC_MODE_PWM1);
  timer_channel_output_shadow_config(TIMER1,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);
	
	timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_2,500);
  timer_channel_output_mode_config(TIMER1,TIMER_CH_2,TIMER_OC_MODE_PWM1);
  timer_channel_output_shadow_config(TIMER1,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);
	
	//timer_channel_output_pulse_value_config(TIMER2,TIMER_CH_4,0);
  //timer_channel_output_mode_config(TIMER2,TIMER_CH_4,TIMER_OC_MODE_PWM1);
  //timer_channel_output_shadow_config(TIMER2,TIMER_CH_4,TIMER_OC_SHADOW_DISABLE);
	
  
	
	/* auto-reload preload enable */
   timer_auto_reload_shadow_enable(TIMER1);
/* auto-reload preload enable */
   timer_enable(TIMER1);
  /* USER CODE BEGIN TIM2_Init 2 */

  

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

	 timer_parameter_struct timer_initpara;
	
	
	
  timer_deinit(TIMER3);
  timer_internal_clock_config(TIMER3);
	timer_struct_para_init(&timer_initpara);
 /* TIMER1 configuration */
 timer_initpara.prescaler         = 2;
 timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
 timer_initpara.counterdirection  = TIMER_COUNTER_UP;
 timer_initpara.period            = 1000;
 timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
 timer_init(TIMER1, &timer_initpara);

 timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);
 timer_interrupt_enable(TIMER3, TIMER_INT_UP);


}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

	 timer_parameter_struct timer_initpara;
	
	rcu_periph_clock_enable(RCU_TIMER3);
	
  timer_deinit(TIMER3);
  timer_internal_clock_config(TIMER3);
	//timer_struct_para_init(&timer_initpara);
	/* TIMER1 configuration */
	timer_initpara.prescaler         = 4;
	timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection  = TIMER_COUNTER_UP;
	timer_initpara.period            = 800;
	timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
	timer_init(TIMER3, &timer_initpara);

	timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);
	timer_interrupt_enable(TIMER3, TIMER_INT_UP);
	//timer_auto_reload_shadow_enable(TIMER3);
	//timer_enable (TIMER3);
	
}
