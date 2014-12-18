//Capture analog b/w PAL video into STM32F4 RAM
//Resolution 366*240
//Use ST-Link for reading picture from memory
//By ILIASAM

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc_control.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

__IO uint8_t UserButtonPressed = 0;
extern volatile uint8_t capture_enabled;
extern volatile uint8_t lines_captured;//number of captured lines

volatile uint16_t frame_buffer[LINES_NUMBER][LINE_BUFFER_SIZE];
uint8_t *line_proc_pointer = (uint8_t*)&frame_buffer;

uint8_t lines_processed = 0; 

/* Private function prototypes -----------------------------------------------*/
void Delay_ms(uint32_t ms);

/* Private functions ---------------------------------------------------------*/

int main(void)
{
  uint8_t frames_num = 0;
  uint8_t i;
  
  
  RCC_ClocksTypeDef RCC_Clocks;
  
  /* Initialize LEDs and User_Button on STM32F4-Discovery --------------------*/
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI); 
  
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);

  //init_clk();
  RCC_GetClocksFreq(&RCC_Clocks);
  
  ADC_DeInit();
  init_tim2();
  adc_init();
  init_adc3();
  
  STM_EVAL_LEDOff(LED6);
  Delay_ms(10);
    
  //while(frames_num<50)
  while(1) //endless - no image process
  {
    asm("nop");
    if (capture_enabled == 0)
    {       
        Delay_ms(10);
        start_capture();
        frames_num++;
    }
  }
  
  Delay_ms(50);
  
  for (i=0;i<lines_captured;i++)
  {
    process_line(i);
  }
  
  
  while(1){}
  
}



//поиск синхросигнала, преобразование уровня черного
//search for sync, shift black level
void process_line(uint8_t line_num)
{
  uint16_t i;
  uint8_t *loc_line_pointer = line_proc_pointer + line_num*(LINE_BUFFER_SIZE*2);//start of this line
  uint16_t shift = 0;
  uint8_t tr_value = loc_line_pointer[3] + 7;
  
  volatile static uint8_t tmp_dat[LINE_BUFFER_SIZE*2];
  for (i=0;i<LINE_BUFFER_SIZE*2;i++) {tmp_dat[i] = loc_line_pointer[i];}
  
  for (i=3;i<65;i++)
  {
    if (loc_line_pointer[i] > tr_value) {break;}
  }
  tr_value = loc_line_pointer[i+5]+2;//black level
  shift = i-8+35;//skip blank field
  
  for (i=0;i<319;i++)
  {
    loc_line_pointer[i] = (loc_line_pointer[i+shift] - tr_value)*2;//*2 - increase brightness
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

