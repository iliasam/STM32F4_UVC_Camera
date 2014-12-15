
/* Includes ------------------------------------------------------------------*/
#include "main.h"
//Send static NV12 image to PC by UVC
//By ILIASAM

#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_video_core.h"


__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

__IO uint8_t UserButtonPressed = 0;


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
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 10);
 
  
    USBD_Init(&USB_OTG_dev,      
            USB_OTG_FS_CORE_ID,
            &USR_desc, 
            &VIDEO_cb,
            &USR_cb);

    while(1)
    {

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


