Some USB Device UVC projects for STM32F4-DISCOVERY:  

More information in Russian: http://geektimes.ru/post/255316/ 

* iar_uvc_mjpeg/iar_uvc_camera - Analoge Video (PAL) captured, encoded to JPEG and transmitted to PC by USB UVC.  
* iar_uvc_nv12/iar_uvc_test - Send static NV12 image to PC by UVC.  
* iar_uvc_mjpeg_static/iar_uvc_mjpeg - Static bmp picture encoded to JPEG and transmitted to PC by USB UVC.  
It is possible to swithch between FLASH (bmp image) and RAM (moving circle) as an image source for JPEG encoder.   
* iar_adc_test - Capture analog b/w PAL video into STM32F4 RAM. 

Примеры USB Device UVC для STM32F4-DISCOVERY:  
* iar_uvc_mjpeg/iar_uvc_camera - Аналоговое видео захватывается, кодируется в JPEG и передается на компьютер с использованием USB UVC.  
* iar_uvc_nv12/iar_uvc_test - Передача статической картинки на компьютер с использованием USB UVC.  
* iar_uvc_mjpeg_static/iar_uvc_mjpeg - Статическое bmp изображение кодируется в JPEG и передается на компьютер с использованием USB UVC.  
* iar_adc_test - Захват аналогового черно-белого сигнала в память STM32F4.

If you heed HOST code variant (to capture data from USB camera), see my another repo: https://github.com/iliasam/STM32_HOST_UVC_Camera
