

#ifndef __ADC_CONTROL_H
#define __ADC_CONTROL_H

#define LINE_BUFFER_SIZE (uint16_t)183   //183*2 samples
#define LINE_PIX_LENGTH (uint16_t)(LINE_BUFFER_SIZE*2)   //183*2 samples
#define LINES_NUMBER 240

#define LOW_ADC_THRESHOLD 40//óðîâåíü ñèíõðîíèçàöèè (ìèíèìàëüíûé ñèãíàë)
#define HIGH_ADC_THRESHOLD 45


void init_clk(void);
void adc_init(void);
void init_tim2(void);
void init_adc3(void);

void stop_capture(void);
void start_capture(void);

void capture_dma_start(void);



#endif 

