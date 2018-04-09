#include "brdTimer.h"

void BRD_Timer_InitStructDef(TIMER_CntInitTypeDef *TimerInitStruct, uint32_t Pll_Mul, uint32_t freq)
{
  TIMER_CntStructInit (TimerInitStruct);
  TimerInitStruct->TIMER_IniCounter       = 0;                                      // Начальное значение таймера
  TimerInitStruct->TIMER_Prescaler        = Pll_Mul;                                      // Предделитель частоты
  TimerInitStruct->TIMER_Period           = CALC_PERIOD(SystemCoreClock, freq, 1);  // Период таймера 1KHz (от HSI = 8МГц) = 8000
  TimerInitStruct->TIMER_CounterMode      = TIMER_CntMode_ClkFixedDir;              // Режим счета
  TimerInitStruct->TIMER_CounterDirection = TIMER_CntDir_Up;                        // Направление счета
}


void BRD_Timer_Init(Timer_Obj *TimerObj, TIMER_CntInitTypeDef *TimerInitStruct)
{
  // Включение тактирования таймера
  RST_CLK_PCLKcmd (TimerObj->ClockMask, ENABLE);

  // Деинициализация таймера
  TIMER_DeInit (TimerObj->TIMERx);

  // Выбор предделителя тактовой частоты таймера 
  TIMER_BRGInit (TimerObj->TIMERx, TimerObj->ClockBRG);

 // Инициализация таймера
  TIMER_CntInit (TimerObj->TIMERx, TimerInitStruct);
}

void BRD_Timer_Start(Timer_Obj *TimerObj)
{
  TIMER_Cmd (TimerObj->TIMERx, ENABLE);
}

void BRD_Timer_InitIRQ(Timer_Obj *TimerObj, uint32_t priority)
{
  // Выбор источников прерываний
  TIMER_ITConfig (TimerObj->TIMERx, TimerObj->EventIT, ENABLE);
  
  // Назначение приоритета аппаратных прерываний от таймера
  NVIC_SetPriority (TimerObj->IRQn, 1);

  // Включение аппаратных прерываний от таймера 
  NVIC_EnableIRQ (TimerObj->IRQn);
}

//// Обработчик аппаратных прерываний от таймера TIMER1
//void Timer1_IRQHandler (void)
//{
//  // Сброс прерывания от таймера TIMER1
//  TIMER_ClearITPendingBit (MDR_TIMER1, TIMER_STATUS_CNT_ARR);
//}
