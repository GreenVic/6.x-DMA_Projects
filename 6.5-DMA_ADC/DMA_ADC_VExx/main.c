#include "brdClock.h"
#include "brdLED.h"
#include "brdADC.h"
#include "brdDMA.h"
#include "brdUtils.h"

#include "brdADC_Select.h"
#include "brdDMA_SelectADC.h"

#define  LED_OK           BRD_LED_1
#define  LED_FAULT        BRD_LED_2
#define  LED_SHOW_PERIOD  2000000

#define  DATA_COUNT    10
uint32_t data_dma[DATA_COUNT];
uint32_t dmaCtrlStart;
uint32_t completedIRQ;

#define DMA_CHANNEL     DMA_Channel_ADC1

#define VALUE_DEF    7

void ADC1_Initialize(void);
void FillData(uint32_t value);

int main(void)
{
  uint32_t i, ADC_DataCnt;
  volatile uint32_t dmaCtrl;
  
  // Clock
  BRD_Clock_Init_HSE_PLL(5);
  //  LED
  BRD_LEDs_Init();
  
  //  DMA
  BRD_DMA_Init();
  
  DMA_DataCtrl_Pri.DMA_SourceBaseAddr = (uint32_t)&MDR_ADC->ADC1_RESULT;
  DMA_DataCtrl_Pri.DMA_DestBaseAddr   = (uint32_t)&data_dma;
  DMA_DataCtrl_Pri.DMA_CycleSize      = DATA_COUNT;
  BRD_DMA_Init_Channel (DMA_CHANNEL, &DMA_ChanCtrl);
  BRD_DMA_InitIRQ(1);
  //  for restart DMA Cycle
  dmaCtrlStart = BRD_DMA_Read_ChannelCtrl(DMA_CHANNEL);  
  
  //  ADC
  ADC1_Initialize();

  while (1)
  {
  //  Run Meas Cycle
    completedIRQ = 0;
    FillData(VALUE_DEF);
    
    // Restart DMA
    BRD_DMA_Write_ChannelCtrl(DMA_CHANNEL, dmaCtrlStart);
    DMA_Cmd(DMA_CHANNEL, ENABLE);
    
    //  Run ADC 
    BRD_ADC1_RunSample(1);
    
    //  Wait meas cycle
    while(!completedIRQ){};

  //  Check Data
    ADC_DataCnt = 0;
    for (i = 0; i < DATA_COUNT; ++i)
      if (data_dma[i] != VALUE_DEF)
        ++ADC_DataCnt;
      
  // Show status to LEDs
    if (ADC_DataCnt == DATA_COUNT)
      BRD_LED_Switch(LED_OK);
    else
      BRD_LED_Set(LED_FAULT, 1);
    Delay(LED_SHOW_PERIOD);
  } 
}

void DMA_IRQHandler(void)
{ 
  //  Останавливаем непрерывное измерение АЦП
  BRD_ADC1_RunSample(0);
  BRD_ADC1_RunSingle(0);
  //  Вычитываем данные, чтобы снять запрос sreq к DMA  
  ADC1_GetResult();
  
  completedIRQ = 1;
  
  NVIC_ClearPendingIRQ (DMA_IRQn);
}

void ADC1_Initialize(void)
{
  ADC_InitTypeDef  ADCInitStruct;
  ADCx_InitTypeDef ADCxInitStruct;
  
  //  ADCs Pin Init
  BRD_ADC_PortInit(BRD_ADC_CH_CLOCK, BRD_ADC_CH_PORT, BRD_ADC_CH_PIN);
  BRD_ADCs_InitStruct(&ADCInitStruct);
   
  BRD_ADCs_Init(&ADCInitStruct);
    
  //  ADC1 Init
  BRD_ADCx_InitStruct(&ADCxInitStruct);
  //ADCxInitStruct.ADC_SamplingMode = ADC_SAMPLING_MODE_CICLIC_CONV;
  BRD_ADC1_Init(&ADCxInitStruct);
  //BRD_ADC1_InitIRQ_EndConv(); - Прерывание не используется
}

void FillData(uint32_t value)
{
  uint32_t i;
  for (i = 0; i < DATA_COUNT; ++i)
  {
    data_dma[i] = value;
  }
}
