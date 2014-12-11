//Capture Analog Video (PAL), make MJPEG encoding and send result to PC by UVC
//By ILIASAM

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_video_core.h"
#include "jprocess.h"
#include "adc_control.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

__IO uint8_t UserButtonPressed = 0;

volatile uint16_t frame_buffer[LINES_NUMBER][LINE_BUFFER_SIZE];
uint8_t *line_proc_pointer = (uint8_t*)&frame_buffer;

//JPEG and USB transmission
uint16_t last_jpeg_frame_size = 0;
volatile uint8_t jpeg_encode_done = 0;//1 - encode stopped //кодирование закончилось
volatile uint8_t new_frame_cap_enabled = 1;//1 - capture and encoding enabled

//CAPTURE
extern volatile uint8_t capture_enabled;
extern volatile uint8_t lines_captured;//number of captured lines

int main(void)
{
  uint8_t i;
  
  RCC_ClocksTypeDef RCC_Clocks;
  
  /* Initialize LEDs and User_Button on STM32F4-Discovery --------------------*/
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO); 
  
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
  
  
  RCC_GetClocksFreq(&RCC_Clocks);
  //SysTick_Config(RCC_Clocks.HCLK_Frequency / 10);
  
  USBD_Init(&USB_OTG_dev,      
            USB_OTG_FS_CORE_ID,
            &USR_desc, 
            &VIDEO_cb,
            &USR_cb);
  
  init_tim2();
  adc_init();
  init_adc3();
  
  Delay_ms(10);
  
  while(1)
  {
    if (new_frame_cap_enabled == 1)//можно начинать захват и кодирование //capture end encoding can be started
    {
      while (capture_enabled == 1){}
      
      new_frame_cap_enabled = 0;
      jpeg_encode_done = 0;
      start_capture();
      //GPIOD->ODR|= GPIO_Pin_15;//led6
      
      while (capture_enabled != 0) {asm("nop");}//ожидание окончания захвата кадра //wait for frame capture end
      for (i=0;i<240;i++) {process_line(i);}
      
      //GPIOD->ODR&= ~GPIO_Pin_15;
    
      GPIOD->ODR|= GPIO_Pin_13;//led3
      last_jpeg_frame_size = jprocess();//do JPEG coding
      GPIOD->ODR&= ~GPIO_Pin_13;//led3
      
      jpeg_encode_done = 1;//encoding ended //кодирование закончено
      //STM_EVAL_LEDToggle(LED3);
    }
  }
  
  
}

//search for sync, shift black level
//поиск синхросигнала и смкщение уровня черного
void process_line(uint8_t line_num)
{
  uint16_t i;
  uint8_t *loc_line_pointer = line_proc_pointer + line_num*(LINE_BUFFER_SIZE*2);//start of this line
  uint16_t shift = 0;
  uint8_t tr_value = loc_line_pointer[3] + 7;
  
  for (i=3;i<65;i++)
  {
    if (loc_line_pointer[i] > tr_value) {break;}
  }
  tr_value = loc_line_pointer[i+5]+2;//black level value
  shift = i-8+35;//skip blank field
  
  for (i=0;i<319;i++)
  {
    loc_line_pointer[i] = (loc_line_pointer[i+shift] - tr_value)*2;// *2  - increase brightness
  }
  
}

void Delay_ms(uint32_t ms)
{
        volatile uint32_t nCount;
        RCC_ClocksTypeDef RCC_Clocks;
        RCC_GetClocksFreq (&RCC_Clocks);
        nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
        for (; nCount!=0; nCount--);
}





