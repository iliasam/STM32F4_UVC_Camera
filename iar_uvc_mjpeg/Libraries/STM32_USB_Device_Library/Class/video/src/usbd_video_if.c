
/* Includes ------------------------------------------------------------------*/
#include "usbd_video_core.h"
#include "usbd_video_if.h"


static uint8_t  VideoInit         (void);
static uint8_t  VideoDeInit       (void);
static uint8_t  VideoCmd     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
static uint8_t  VideoGetState     (void);


VIDEO_FOPS_TypeDef  AUDIO_OUT_fops =
{
  VideoInit,
  VideoDeInit,
  VideoCmd,
  VideoGetState
};


static uint8_t  VideoInit   (void)
{
    
  return 1;
}


static uint8_t  VideoDeInit (void)
{
  
  return 1;
}


static uint8_t  VideoCmd(uint8_t* pbuf,
                         uint32_t size,
                         uint8_t cmd)
{

  
  return 1;
}


static uint8_t  VideoGetState   (void)
{
  return 1;
}

