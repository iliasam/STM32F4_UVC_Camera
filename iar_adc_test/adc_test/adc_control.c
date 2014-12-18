#include "stm32f4xx.h"
#include "adc_control.h"
#include "stm32f4_discovery.h"
#include "bitbanding.h"

#define ADC_CDR_ADDRESS    ((uint32_t)0x40012308)

volatile uint8_t capture_enabled = 0;
volatile uint8_t sync_flag = 0;//0 - ожидание спада, 1 - ожидание возрастания 
//0- wait fow low edge 1 -wait for high edge

volatile uint8_t frame_sync_begin = 0;

extern volatile uint16_t frame_buffer[LINES_NUMBER][LINE_BUFFER_SIZE];

volatile uint16_t *capture_pointer;
volatile uint8_t lines_captured = 0;//number of captured lines

//This adc used for sync detection
//Этот АЦП используется для обнаружения синхросигнала
void init_adc3(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  NVIC_InitTypeDef      NVIC_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
  
    /* ADC Common configuration *************************************************/
  //dual mode 1&2
  
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_8b;

  /* ADC1 regular channel 12 configuration ************************************/
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;	//without this zero only result
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 0;
  ADC_Init(ADC1, &ADC_InitStructure);
  /* ADC1 regular channel12 configuration */
  ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_3Cycles); 

  /* ADC2 regular channel 12 configuration ************************************/
  ADC_Init(ADC3, &ADC_InitStructure);
  
  ADC_AnalogWatchdogThresholdsConfig(ADC3, 255,LOW_ADC_THRESHOLD);//high, low
  ADC_AnalogWatchdogCmd(ADC3, ADC_AnalogWatchdog_AllRegEnable);
  
  //NVIC_EnableIRQ(ADC_IRQn);                // Enable IRQ for ADC in NVIC 
  
  NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  ADC_ITConfig(ADC3, ADC_IT_AWD,ENABLE);

  ADC_Cmd(ADC3, ENABLE);

  /* Start ADC1 Software Conversion */
  ADC_SoftwareStartConv(ADC3);
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}

//ADC1 & ADC2 for image capture
void adc_init(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef DMA_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef      NVIC_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE);
  
  /* Configure ADC Channel 12 pin as analog input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* DMA2 Stream0 channel0 configuration */
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC_CDR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&frame_buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = LINE_BUFFER_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 10;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
   
  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
  
  /* DMA2_Stream0 enable */
  DMA_Cmd(DMA2_Stream0, ENABLE);

  /* ADC Common configuration *************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_DualMode_Interl;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_7Cycles;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_3;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInit(&ADC_CommonInitStructure);  
  
  /* DMA mode 3 is used in interleaved mode in 6-bit or 8-bit resolutions */
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_8b;

  /* ADC1 regular channel 12 configuration ************************************/
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;	//without this zero only result
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1, &ADC_InitStructure);
  /* ADC1 regular channel12 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_3Cycles); 

  /* ADC2 regular channel 12 configuration ************************************/
  ADC_Init(ADC2, &ADC_InitStructure);
  /* ADC2 regular channel12 configuration */
  ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);

  /* Enable DMA request after last transfer (multi-ADC mode) ******************/
  ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);

  /* Enable ADC1 **************************************************************/
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC2 **************************************************************/
  ADC_Cmd(ADC2, ENABLE);

  /* Start ADC1 Software Conversion */
  ADC_SoftwareStartConv(ADC1);
}


void ADC_IRQHandler(void)      
{
  uint16_t curr_time = 0;
  static uint16_t prev_time = 0;
  volatile uint16_t pulse_period = 0;
  static uint16_t lines_cnt = 0;
  //GPIOD->ODR ^= GPIO_Pin_13;
  
  if (sync_flag == 0)
  {
    ADC_AnalogWatchdogThresholdsConfig(ADC3, HIGH_ADC_THRESHOLD,0);//срабатывает при >42
    //начался спад
    //low edge begin
    curr_time = TIM2->CNT;
    if (curr_time > prev_time){pulse_period = (curr_time - prev_time);}
    else {pulse_period = (0xFFFF - prev_time + curr_time);}
    
    if ((pulse_period > 62) && (pulse_period <66) && (capture_enabled == 2))
    {
      if (frame_sync_begin == 1)
      {
        //синхроимпульсы закончились, пошли пустые строки
        //frame sync ended
        //подготовка к захвату изображения
        //prepare to capture
        frame_sync_begin = 0;
        lines_cnt = 0;
        capture_pointer = (uint16_t*)&frame_buffer;
      }
      else 
      {
        lines_cnt++;
        //if (lines_cnt == 17) {GPIOD->ODR ^= GPIO_Pin_15;}//toggle led - every 17 line
        if ((lines_cnt >= 17) && (lines_cnt < (17+240))) 
        {
          capture_dma_start();
        }
        else if (lines_cnt >= (17+240)) stop_capture();
      }
    }
    else if ((pulse_period > 30) && (pulse_period <34))
    {
      //кадровые синхроимпульсы
      //frame sync signal
      frame_sync_begin = 1;
      capture_enabled = 2;
    }
    
    prev_time = curr_time;
    sync_flag = 1;
  }
  else
  {
    ADC_AnalogWatchdogThresholdsConfig(ADC3, 255,LOW_ADC_THRESHOLD);//срабатывает при близости к 0
    sync_flag = 0;
  }
  
  //ADC_ClearITPendingBit(ADC3, ADC_IT_AWD); ////PROBLEM WITH INTERRUPTS!
  BIT_BAND_PER(ADC3->SR,ADC_IT_AWD)=RESET;
}

void DMA2_Stream0_IRQHandler(void)
{
  GPIOD->ODR&= ~GPIO_Pin_15;
  if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))
  {
    DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
    ADC_Cmd(ADC1, DISABLE);
    ADC_Cmd(ADC2, DISABLE);
    lines_captured++;
  }
  asm("nop");
}

void capture_dma_start(void)
{
  DMA_Cmd(DMA2_Stream0, DISABLE);
  DMA2_Stream0->NDTR = LINE_BUFFER_SIZE;
  DMA2_Stream0->M0AR = (uint32_t)capture_pointer;
  capture_pointer+= LINE_BUFFER_SIZE;
  
  DMA_Cmd(DMA2_Stream0, ENABLE);
  
  //adc was disabled
  ADC_Cmd(ADC2, ENABLE);
  ADC_Cmd(ADC1, ENABLE);
  ADC_SoftwareStartConv(ADC1);
  GPIOD->ODR|= GPIO_Pin_15;
}

void start_capture(void)
{
  capture_enabled = 1;
  sync_flag = 0;
  frame_sync_begin = 0;
  lines_captured = 0;
  ADC_AnalogWatchdogThresholdsConfig(ADC3, 255,LOW_ADC_THRESHOLD);//срабатывает при близости к 0
  ADC_Cmd(ADC3, ENABLE);
  ADC_SoftwareStartConv(ADC3);
}

void stop_capture(void)
{
  capture_enabled = 0;
  ADC_Cmd(ADC3, DISABLE);
}




//для счета времени синхроимнульсов
//дискретность - 1us
void init_tim2(void)
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;
  
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
 
  TIM_TimeBaseStructure.TIM_Prescaler = 83;//83+1=84
  TIM_TimeBaseStructure.TIM_Period = 0xffff;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 
 
  
  TIM_OCStructInit(&TIM_OCInitStructure);
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0xffff;//PWM DUTY
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC2Init(TIM2,&TIM_OCInitStructure);
 
  TIM_OC2Init(TIM2,&TIM_OCInitStructure);  
  TIM_Cmd(TIM2, ENABLE);
}