#include "brdClock.h"
#include "brdLED.h"
#include "brdADC.h"
#include "brdDMA.h"
#include "brdUtils.h"

#include "brdDMA_SelectADC.h"

ADCx_InitTypeDef ADCxInitStruct;

#define ADC_CHANNEL     4
#define ADC_FIFO_EN     FIEN4

#define DMA_CHANNEL     0
#define DMA_ADC1_SREQ   60

#define DATA_COUNT  10
uint32_t data_dma[DATA_COUNT];
uint32_t data_rd[DATA_COUNT];
uint32_t dmaCtrlStart;
uint32_t completedIRQ;

#define LED_OK            BRD_LED_1
#define LED_FAULT         BRD_LED_2
#define DELAY_LED_PERIOD  1000000

void FillData(uint32_t value);

ADCxControl logADC1_start, logADC1_IRQ;
DMAControl  logDMA_start, logDMA_IRQ;


int main(void)
{ 
  uint32_t i;
  volatile uint32_t dmaCtrl;
  
  // Clock
  BRD_Clock_Init_HSE_PLL(5);
  //  LED
  BRD_LEDs_Init();
    
  //  DMA
  BRD_DMA_Init();
  MDR_DMA->CHNL_REQ_MASK_SET = 0xFFFFFFFF; 
  MDR_DMA->CHNL_ENABLE_SET = 0xFFFFFFFF;
  
  DMA_DataCtrl_Pri.DMA_SourceBaseAddr = (uint32_t)&ADC1->RESULTCH_xx[ADC_CHANNEL];  //   RESULT;
  DMA_DataCtrl_Pri.DMA_DestBaseAddr   = (uint32_t)&data_dma;
  DMA_DataCtrl_Pri.DMA_CycleSize      = DATA_COUNT - 2;
  BRD_DMA_Init_Channel (DMA_CHANNEL, &DMA_ChanCtrl);
  //  Assign ADC1_SREQ to DMA channel_0
  MDR_DMA->CHMUX0 = DMA_ADC1_SREQ;
  //  for restart
  dmaCtrlStart = BRD_DMA_Read_ChannelCtrl(DMA_CHANNEL);
    
  //  ADC
  BRD_ADCx_InitStruct(&ADCxInitStruct);
  ADCxInitStruct.ADC_FIFOEN_0_31 = ADC_FIFO_EN;
  ADCxInitStruct.ADC_PAUSE = 0x3F;  
  BRD_ADC1_Init(&ADCxInitStruct);
  ADCx_SetChannel(ADC1, ADC_CHANNEL);

//  //  ADC IRQ Enable - test  
//  ADC1->CONFIG2 |= 1;   // ADC IRQ - Not Empty
//  NVIC_EnableIRQ(ADC1_IRQn);

  while (1)
  {
    // Fill data with a value
    FillData(7);
    
    //  DMA SREQ Enable
    ADC1->DMAREQ = (1 << 24) | ADC_CHANNEL;
    
    BRD_ADCx_LogReg(ADC1, &logADC1_start);
    BRD_DMA_ReadRegs(&logDMA_start);
    
    //  Run single ADC measurement    
    completedIRQ = 0;
    //ADCx_Start(ADC1);
    for (i = 0; i < DATA_COUNT-2; ++i)
      BRD_ADCx_StartAndWaitCompleted(ADC1);
    
    //  Wait DMA IRQ
    while(!completedIRQ)
    { // update reg values for debug
//      BRD_ADCx_LogReg(ADC1, &logADC1_IRQ);
//      dmaCtrl = BRD_DMA_Read_ChannelCtrl(DMA_CHANNEL);
    }

    // Led status
    BRD_LED_Switch(LED_OK);
    Delay(DELAY_LED_PERIOD);
    
    //  Restore ADC1 Enable
//    ADC1->CONFIG0 |= 1;  // ENA
  }
}

void DMA_DONE0_Handler(void)
{ 
  BRD_ADCx_LogReg(ADC1, &logADC1_IRQ);
  BRD_DMA_ReadRegs(&logDMA_IRQ);  

  //  Try to clear active DMA SREQ
//  ADC1->CONFIG0 &= 0xFFFFFFFE;  // ENA clr
  ADC1->DMAREQ = 0;
  
  // Новый цикл DMA, если текущий закончился
  if (BRD_DMA_Read_ChannelCtrl(DMA_CHANNEL) != dmaCtrlStart)
    BRD_DMA_Write_ChannelCtrl(DMA_CHANNEL, dmaCtrlStart);

  completedIRQ = 1;
  
  NVIC_ClearPendingIRQ (DMA_IRQn);
}

void INT_ADC1_Handler(void)
{
  uint32_t i = 0;
  volatile uint32_t cnt = 0;
  while (ADC1->STATUS & 1)
  { 
    if (i < DATA_COUNT)
      data_rd[i] = ADC1->RESULT;
    ++cnt;
  }  
  
  completedIRQ = 1;
  
  NVIC_ClearPendingIRQ (ADC1_IRQn);
}  

void FillData(uint32_t value)
{
  uint32_t i;
  for (i = 0; i < DATA_COUNT; ++i)
  {
    data_dma[i] = value;
    data_rd[i] = value;
  }
}


