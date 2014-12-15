//Stactic bmp picture encoded to JPEG and transmitted to PC by USB UVC.
//by ILIASAM

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_video_core.h"
#include "math.h"
#include "jprocess.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

__IO uint8_t UserButtonPressed = 0;

uint8_t raw_image[240][320];

uint16_t last_jpeg_frame_size = 0;
volatile uint8_t jpeg_encode_done = 0;//1 - encode stopped //кодирование закончилось
volatile uint8_t jpeg_encode_enabled = 1;//1 - capture and encoding enabled //кодирование разрешено

double circle_x = 0;
double circle_y = 0;
double angle = 0;
uint8_t color;

int main(void)
{
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
  
  while(1)
  {
    if (jpeg_encode_enabled == 1)//можно начинать кодирование
    {
      jpeg_encode_enabled = 0;
      jpeg_encode_done = 0;
      last_jpeg_frame_size = jprocess();
      
      circle_x = 160 + sin(angle)*60;
      circle_y = 120 + cos(angle)*60;
      angle+= 0.05;
      color+=10;
      
      draw_circle((int)circle_x, (int)circle_y, 15,color);
      jpeg_encode_done = 1;//encoding ended //кодирование закончено
      STM_EVAL_LEDToggle(LED3);
    }
    
    
  }
  
  while(1){
    asm("nop");
  }
  
}


//внимание - нет проверок на выход за края вдеопамяти
void draw_circle(int Hcenter, int Vcenter, int radius,uint8_t color)
{
    int x = radius;
    int y = 0;
    int xChange = 1 - (radius << 1);
    int yChange = 0;
    int radiusError = 0;
    int i;
    int p = 3 - 2 * radius;

    while (x >= y)
    {
        for (i = Hcenter - x; i <= Hcenter + x; i++)
        {
        	raw_image[Vcenter + y][i] = color;
        	raw_image[Vcenter - y][i] = color;
        }
        for (i = Hcenter - y; i <= Hcenter + y; i++)
        {
        	raw_image[Vcenter + x][i] = color;
        	raw_image[Vcenter - x][i] = color;
        }

        y++;
        radiusError += yChange;
        yChange += 2;
        if (((radiusError << 1) + xChange) > 0)
        {
            x--;
            radiusError += xChange;
            xChange += 2;
        }
    }
}



void delay_ms(uint32_t ms)
{
        volatile uint32_t nCount;
        RCC_ClocksTypeDef RCC_Clocks;
        RCC_GetClocksFreq (&RCC_Clocks);
        nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
        for (; nCount!=0; nCount--);
}





