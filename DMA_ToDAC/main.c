#include "brdClock.h"
#include "brdLed.h"
#include "brdDMA.h"
#include "brdUtils.h"
#include "brdTimer.h"
#include "brdTimer_Select.h"
#include "brdDAC.h"
#include "brdMath.h"
#include "brdDMA_SelectDAC.h"

//  ОШИБКИ
//#define BUG_SSPx_ACTIVATE


//  Clock
#define  PLL_MUX           RST_CLK_CPU_PLLmul10

//  Индикация работы
#define  LED_DMA_CYCLE     BRD_LED_1
#define  LED_DMA_ON_INIT   BRD_LED_2

//  Различия при инициализвации DMA
#ifdef USE_MDR1986VE9x
 
  #define  DAC_PIN_INIT   BRD_DAC2_PortInit()
  #define  DAC_ENABLE     BRD_DAC2_SetEnable(ENABLE)
  #define  DAC_REG_DATA   MDR_DAC->DAC2_DATA

  #define  DMA_CHANNEL         DMA_Channel_TIM1
  #define  TIMER_CALL_DMA_ENA  TIMER_DMACmd (MDR_TIMER1, TIMER_STATUS_CNT_ARR, ENABLE)
  
  #define  DMA_CLOCK_FIX  (RST_CLK_PCLK_SSP1 | RST_CLK_PCLK_SSP2 | RST_CLK_PCLK_DMA)
  
#elif defined (USE_MDR1986VE1T)

  #define  DAC_PIN_INIT   BRD_DAC1_PortInit()
  #define  DAC_ENABLE     BRD_DAC1_SetEnable(ENABLE)
  #define  DAC_REG_DATA   MDR_DAC->DAC1_DATA

  #define  DMA_CHANNEL         DMA_Channel_SREQ_TIM1
  
  #define  TIMER_CALL_DMA_ENA  TIMER_DMACmd (MDR_TIMER1, TIMER_STATUS_CNT_ARR, TIMER_DMA_Channel0, ENABLE)
  
  #define  DMA_CLOCK_FIX  (RST_CLK_PCLK_SSP1 | RST_CLK_PCLK_SSP2 | RST_CLK_PCLK_SSP3 | RST_CLK_PCLK_DMA)
#endif

uint32_t DMA_ChannelCtrl;

//  Выходной сигнал ЦАП 1КГц
#define  SIGNAL_FREQ_HZ   1000

//  Массив значений сигнала по оси времени
#define  DATA_COUNT       400
uint16_t signal[DATA_COUNT] __attribute__((section("EXECUTABLE_MEMORY_SECTION"))) __attribute__ ((aligned (4)));

//  vars
uint32_t IrQ_On = 0;

uint32_t CheckDoLedSwitch(void);

int main(void)
{
  TIMER_CntInitTypeDef TimerInitStruct;
  
  //  Jtag Catch protection
  Delay(1000000);
  
  //  Clock CPU
  BRD_Clock_Init_HSE_PLL(PLL_MUX);
  
  //  LED
  BRD_LEDs_Init();
  // LED - Show Initialization status
  BRD_LED_Set(LED_DMA_CYCLE | LED_DMA_ON_INIT, 1);  
  
  //  DAC Init
  BRD_DACs_Init();
  DAC_PIN_INIT;
  DAC_ENABLE;
  
  //  Calc Output Signal
  FillSin(DATA_COUNT, &signal[0], VOLT_TO_DAC(1), VOLT_TO_DAC(2));
  //FillSaw(DATA_COUNT, &signal[0], VOLT_TO_DAC(1), VOLT_TO_DAC(1));
  //FillTriangle(DATA_COUNT, &signal[0], VOLT_TO_DAC(1), VOLT_TO_DAC(1));  
  //FillMeandr(DATA_COUNT, &signal[0], VOLT_TO_DAC(1), VOLT_TO_DAC(1), 0.5);
  
  //  DMA Init
  DMA_DataCtrl_Pri.DMA_SourceBaseAddr = (uint32_t)&signal;
  DMA_DataCtrl_Pri.DMA_DestBaseAddr   = (uint32_t)&DAC_REG_DATA;
  DMA_DataCtrl_Pri.DMA_CycleSize      = DATA_COUNT; 

#ifndef BUG_SSPx_ACTIVATE
  RST_CLK_PCLKcmd (DMA_CLOCK_FIX, ENABLE);
#else
  RST_CLK_PCLKcmd (RST_CLK_PCLK_DMA, ENABLE);
#endif

  BRD_DMA_Init();  
  BRD_DMA_Init_Channel(DMA_CHANNEL, &DMA_ChanCtrl);
  
  BRD_DMA_Read_ChannelCtrl(DMA_CHANNEL, &DMA_ChannelCtrl);  // for fast restart DMA cycle

  //  Timer
  BRD_Timer_InitStructDef(&TimerInitStruct, SIGNAL_FREQ_HZ * DATA_COUNT, 64000); // DAC Out 1KHz
  BRD_Timer_Init(&brdTimer1, &TimerInitStruct);

  //  Timer - DMA start
  TIMER_CALL_DMA_ENA;
  //  Timer Start
  BRD_Timer_Start(&brdTimer1);

  // LED - Clear Initialization status
  BRD_LED_Set(LED_DMA_CYCLE | LED_DMA_ON_INIT, 0);

	while (1)
	{
    if (IrQ_On)
    {
      IrQ_On = 0;
      
      if (CheckDoLedSwitch())
        BRD_LED_Switch(LED_DMA_CYCLE);
    }
	}
}

uint32_t CheckDoLedSwitch(void)
{
  static uint32_t IqrCount = 0;
  
  ++IqrCount;
  if (IqrCount > SIGNAL_FREQ_HZ)
    {
    IqrCount = 0;
    return 1;   //  Do switch Led - 1Hz
  }
  else
    return 0;   //  Do not switch Led
}

void DMA_IRQHandler (void)
{ 
  //  CASE1: Fast Next DMA Cycle Init 
  BRD_DMA_Write_ChannelCtrl(DMA_CHANNEL, DMA_ChannelCtrl);
  DMA_Cmd(DMA_CHANNEL, ENABLE);
 
  //  CASE2: Медленно, дает артефакт сигнала (см. осциллографом) при PLL_MUX < RST_CLK_CPU_PLLmul3 
//  DMA_Init(DMA_CHANNEL, &DMA_ChanCtrl); 
  
  IrQ_On = 1; 
  
  NVIC_ClearPendingIRQ(DMA_IRQn);
}	


