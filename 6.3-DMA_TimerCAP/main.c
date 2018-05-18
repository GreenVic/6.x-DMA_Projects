#include "brdDef.h"

#include "brdClock.h"
#include "brdBtn.h"
#include "brdLCD.h"
#include "brdTimer.h"
#include "brdUtils.h"
#include "brdDMA.h"
#include "brdPort.h"

#include "TaskDefs.h"


/**
  ДЛЯ ЗАПУСКА: На отладочной плате соединить PF7 (XP8 pin 10) <-> PA9 (XP10 pin 25), rev 5
**/

//  Множитель для частоты ядра х10 (80Мгц)
//  На этой же частоте "считают" таймеры
#define  PLL_MUX       RST_CLK_CPU_PLLmul10

//  -----------   Таймер захвата - Timer1 -------
//  Выбор TIMER1 и настройки для его инициализации функцией BRD_Timer_Init()
Timer_Obj brdTimerCap =
{
  .TIMERx    = MDR_TIMER1,
  .ClockMask = RST_CLK_PCLK_TIMER1,
  .ClockBRG  = TIMER_HCLKdiv1,
  .EventIT   = TIMER_STATUS_CNT_ARR,
  .IRQn      = Timer1_IRQn
};

//  Выбор канала таймера захвата
#define TIM_CHANNEL_CAP     TIMER_CHANNEL4            //  4-й канал
#define TIM_REG_CCR         CCR4                      //  CCR4 - Регистр захвата 4-го канала, отсюда DMA читает данные
#define TIM_DMA_SREQ_CCR    TIMER_STATUS_CCR_CAP_CH4  //  Событие захвата в CCR4 генерирует запрос к DMA
#define DMA_CHANNEL_CAP     DMA_Channel_TIM1          //  Канал DMA обслуживающий TIMER1
#define DMA_IRQ_PRIORITY    1                         //  Приоритет прерывания DMA


//  Выбор GPIO - входа для Захвата сигнала от ШИМ
#if defined ( USE_BOARD_VE_91 )  || defined ( USE_BOARD_VE_94 )
  //  Вход PA9 - TMR1_CH4
  brdPort_Obj Port_TimerPinCAP = 
  {
    .PORTx          = MDR_PORTA,
    .Port_ClockMask = RST_CLK_PCLK_PORTA,
    .Port_PinsSel   = PORT_Pin_9,
    .Port_PinsFunc  = PORT_FUNC_ALTER,
    .Port_PinsFunc_ClearMask = 0,                     //  Здесь не используется
    .pInitStruct    = &pinTimerCAP                    //  Настройки пина для захвата по умолчанию - TaskDefs.h
  };
#elif defined ( USE_BOARD_VE_1 )
  //  Вход PE6 - TMR1_CH4
  brdPort_Obj Port_TimerPinCAP = 
  {
    .PORTx          = MDR_PORTE,
    .Port_ClockMask = RST_CLK_PCLK_PORTE,
    .Port_PinsSel   = PORT_Pin_6,
    .Port_PinsFunc  = PORT_FUNC_MAIN,
    .Port_PinsFunc_ClearMask = 0,                     //  Здесь не используется
    .pInitStruct    = &pinTimerCAP                    //  Настройки пина для захвата по умолчанию - TaskDefs.h
  };
#endif 


//  -----------   Таймер ШИМ  - Timer3  -------
//  Выбор TIMER3 и настройки для его инициализации функцией BRD_Timer_Init()
Timer_Obj brdTimerPWM =
{
  .TIMERx    = MDR_TIMER3,
  .ClockMask = RST_CLK_PCLK_TIMER3,
  .ClockBRG  = TIMER_HCLKdiv1,
  .EventIT   = TIMER_STATUS_CNT_ARR,
  .IRQn      = Timer3_IRQn
};

//  Выбор канала таймера для вывода ШИМ
#define TIM_CHANNEL_PWM     TIMER_CHANNEL1          //  1-й канал
#define TIM_PWM_PERIOD      20                      //  Начальный период ШИМ по умолчанию
#define TIM_PWM_WIDTH       3                       //  Длительность импульса ШИМ

//  Выбор GPIO - вывода сигнала ШИМ
#if defined ( USE_BOARD_VE_91 )  || defined ( USE_BOARD_VE_94 )
  //  Вывод PF7 - TMR3_CH1
  brdPort_Obj Port_TimerPinPWM = 
  {
    .PORTx          = MDR_PORTF,
    .Port_ClockMask = RST_CLK_PCLK_PORTF,
    .Port_PinsSel   = PORT_Pin_7,
    .Port_PinsFunc  = PORT_FUNC_OVERRID,
    .Port_PinsFunc_ClearMask = 0,                     //  Здесь не используется
    .pInitStruct    = &pinTimerPWM                    //  Настройки пина для ШИМ по умолчанию - TaskDefs.h
  };
#elif defined ( USE_BOARD_VE_1 )
  //  Вывод PB0 - TMR3_CH1
  brdPort_Obj Port_TimerPinPWM = 
  {
    .PORTx          = MDR_PORTB,
    .Port_ClockMask = RST_CLK_PCLK_PORTB,
    .Port_PinsSel   = PORT_Pin_0,
    .Port_PinsFunc  = PORT_FUNC_OVERRID,
    .Port_PinsFunc_ClearMask = 0,                     //  Здесь не используется
    .pInitStruct    = &pinTimerPWM                    //  Настройки пина для ШИМ по умолчанию - TaskDefs.h
  };
#endif

//  -----------   Переменные и определения для задачи  -------
#if defined ( USE_BOARD_VE_91 )  || defined ( USE_BOARD_VE_94 )
  #define uintCCR_t         uint16_t
  
  //#define DMA_CASE_SREQ_DIS
  
#elif defined ( USE_BOARD_VE_1 )
  #define uintCCR_t         uint32_t
#endif  
  
#ifdef DMA_CASE_SREQ_DIS
  #define PASS_FIRST_DATA   1
#else
  #define PASS_FIRST_DATA   0
#endif
  
#define   DATA_COUNT  16                      //  Количество событий захвата для измерения частоты, равно кол-ву передач в цикле DMA
//  Массив куда DMA будет копировать данные из регистра захвата
uintCCR_t  arrDataCCR[DATA_COUNT] __attribute__((section("EXECUTABLE_MEMORY_SECTION"))) __attribute__ ((aligned (4)));
uintCCR_t  arrPeriod[DATA_COUNT - 1];         //  Массив дельт, между событиями захвата. Для анализа при отладке
uint32_t   passDataCap = PASS_FIRST_DATA;     //  Кол-во первых дельт, пропускаемых при подсчете периода. 
                                              //  Начальные значения битые из-за пересинхронизации частот таймера и DMA при высоких частотах PWM
uint32_t  capPeriodPWM = TIM_PWM_PERIOD;      //  Период ШИМ, меняется кнопками UP/DOWN
uint32_t  dmaChanCtrlStart;                   //  Управляющее слово настроенного канала DMA, используется для перенастройки DMA на новый цикл.

//  Переменные для результата и отладки 
uint32_t cntStart;                            //  Количество запусков измерений, т.е. циклов DMA
uint32_t cntOk;                               //  Количество успешных измерений
uint32_t cntFault;                            //  Количество сбойных измерений
uint32_t cntErr;                              //  Количество ошибок в текущем измерении, по анализу arrPeriod
uint32_t capPeriod;                           //  Значение периода в текущем измерении - усредненное значение по arrPeriod

#define CAP_MEAS_PERIOD  100000               //  Период перезапуска измерений и обновления LCD экрана

uint32_t DoCheckResult;                       //  Флаг на обработку данных, выставляется в прерывании DMA
uint32_t DoWaitAndRunNextMeas;                //  Флаг на выдержку задержки для отображения экрана и запуска нового цикла
uint32_t cntWait;                             //  Счетчик задержки на отображение экрана

//  Стирание предыдущих данных захвата
void ClearCaptureData(uintCCR_t data);
//  Вычисление периода
uint32_t CalcPeriod(uint32_t passOffset, uint32_t * errCnt);

//  Отображение результатов на LCD экране
void LCD_ShowResult(uint32_t periodCap, uint32_t errCnt);
void LCD_ShowSettings(uint32_t periodPWM, uint32_t dataOffs);
//  Обработка кнопок на изменение capPeriodPWM и passDataCap
void ProcessBtnCommands(void);
//  Разрешение sreq к DMA от таймера захвата
void TimerCap_CallDMAEna(FunctionalState enable);

int main(void)
{
  TIMER_CntInitTypeDef    TimerInitStruct;    //  Структура для настройки счетчика в таймерах
  TIMER_ChnInitTypeDef    TimerChanCfg;       //  Структура настройки канала
  TIMER_ChnOutInitTypeDef TimerChanOutCfg;    //  Структура настройки вывода канала
    
  //  Тактирование ядра 80МГц
  BRD_Clock_Init_HSE_PLL(RST_CLK_CPU_PLLmul10);
  
  //  Инициализация кнопок и LCD экрана
  BRD_BTNs_Init();
  BRD_LCD_Init();
  //  Вывод начальных значений ШИМ
  LCD_ShowSettings(capPeriodPWM, passDataCap);

  //  ТАЙМЕР ШИМ
  //  Настройка счетчика таймера ШИМ
  TIMER_CntStructInit (&TimerInitStruct);
  TimerInitStruct.TIMER_Period = capPeriodPWM;  
  BRD_Timer_Init(&brdTimerPWM, &TimerInitStruct);
  //  Настройка канала таймера на вывод ШИМ
  BRD_TimerChannel_InitStructPWM(TIM_CHANNEL_PWM, &TimerChanCfg, &TimerChanOutCfg);
  BRD_TimerChannel_Apply(brdTimerPWM.TIMERx, &TimerChanCfg, &TimerChanOutCfg);
  //  Выставление скважности ШИМ
  TIMER_SetChnCompare (brdTimerPWM.TIMERx , TIM_CHANNEL_PWM, TIM_PWM_WIDTH);
  //  Настройка пина GPIO на вывод от канала таймера ШИМ
  BRD_Port_Init(&Port_TimerPinPWM);
  
  //  ТАЙМЕР ЗАХВАТА
  //  Настройка счетчика таймера захвата
  TIMER_CntStructInit (&TimerInitStruct);
  TimerInitStruct.TIMER_Period     = 0xFFFF;  //  Max period
  BRD_Timer_Init(&brdTimerCap, &TimerInitStruct);  
  //  Настройка канала таймера на захват внешнего сигнала (от ШИМ)
  BRD_TimerChannel_InitStructCAP(TIM_CHANNEL_CAP, &TimerChanCfg, &TimerChanOutCfg);
  BRD_TimerChannel_Apply(brdTimerCap.TIMERx, &TimerChanCfg, &TimerChanOutCfg);
  //  Настройка пина GPIO на вход сигнала для канала таймера захвата
  BRD_Port_Init(&Port_TimerPinCAP);
  
  //  DMA
  //  Настройка канала DMA для передачи данных от регистра захвата - TaskDefs.h
  BRD_DMA_Init();
  DMA_DataCtrl_Pri.DMA_SourceBaseAddr = (uint32_t)&brdTimerCap.TIMERx->TIM_REG_CCR;
  DMA_DataCtrl_Pri.DMA_DestBaseAddr   = (uint32_t)&arrDataCCR;
  DMA_DataCtrl_Pri.DMA_CycleSize      = DATA_COUNT;
#ifdef USE_BOARD_VE_1
  DMA_DataCtrl_Pri.DMA_DestIncSize    = DMA_DestIncWord,
  DMA_DataCtrl_Pri.DMA_MemoryDataSize = DMA_MemoryDataSize_Word,
#endif  
  
  BRD_DMA_Init_Channel(DMA_CHANNEL_CAP, &DMA_ChanCtrl);
  
  BRD_DMA_InitIRQ(DMA_IRQ_PRIORITY);
  
  //  Сохранение управляющего слова канала DMA, для следующих перезапусков
  dmaChanCtrlStart = BRD_DMA_Read_ChannelCtrl(DMA_CHANNEL_CAP);
  //  Заполнение начальных данных любым значением
  ClearCaptureData(3); 

  //  Разрешение запросов sreq к DMA по событию захвата фронта сигнала на входе канала таймера
  TimerCap_CallDMAEna(ENABLE);

  //  Запуск таймеров. 
  //    Вывод ШИМ выдает сигнал на вход для захвата, по внешнему подключнию пинов проводами на демо-плате (PF7 <-> PA9).
  //    DMA сохраняет отсчеты захвата в массив arrDataCCR.
  //    По окончании цикла DMA возникает прерывание - DMA_IRQHandler(), 
  //    в котором выставлется запрос на обработку данных DoCheckResult = 1.
  BRD_Timer_Start(&brdTimerPWM);
  BRD_Timer_Start(&brdTimerCap);


  //  Основной цикл
  while (1)
  {
    //  Вычислние результатат и отображение на LCD
    if (DoCheckResult)
    {
      capPeriod = CalcPeriod(passDataCap, &cntErr);
      if (cntErr)
        ++cntFault;
      else
        ++cntOk;

      LCD_ShowResult(capPeriod, cntErr);
      
      DoCheckResult = 0;
      DoWaitAndRunNextMeas = 1;
      cntWait = CAP_MEAS_PERIOD;
    }
    
    //  Отсчет задержки на отображение LCD и запуск следующего измерения
    if (DoWaitAndRunNextMeas)
      if (!(--cntWait))
      {
        DoWaitAndRunNextMeas = 0;
       
        //  Очистка предыдущих данных
        ClearCaptureData(3);

        //  Сброс счетчика, чтобы не обрабатывать ситуацию переполнения счета с 0xFFFF в 0x0000
        TIMER_SetCounter(brdTimerCap.TIMERx, 0);
        
#ifdef DMA_CASE_SREQ_DIS
        
        //  Перезапуск канала DMA
        BRD_DMA_Write_ChannelCtrl(DMA_CHANNEL_CAP, dmaChanCtrlStart);
        DMA_Cmd (DMA_CHANNEL_CAP, ENABLE);
        //  Разрешение запросов от событий захвата к каналу DMA
        TIMER_DMACmd (brdTimerCap.TIMERx, TIM_DMA_SREQ_CCR, ENABLE);
        //  Разрешение обработки одиночных запросов (sreq) к каналу DMA
        MDR_DMA->CHNL_USEBURST_CLR |= 1 << DMA_CHANNEL_CAP;
        
#else
        //  Разрешение запросов от событий захвата к каналу DMA
        TimerCap_CallDMAEna(ENABLE);

#endif         

        ++cntStart;    
      }
      else //  Обработка нажатия кнопок и изменение настроек
        ProcessBtnCommands();
  }
}

void DMA_IRQHandler (void)
{
  volatile uint32_t val;
  
  //  Запрет запросов от событий захвата к каналу DMA
  TimerCap_CallDMAEna(DISABLE);

#ifdef DMA_CASE_SREQ_DIS
  
  //  Несмотря на запрет запросов от таймера, прерывания все-равно генерируются. Даже если замаскировать канал.
  //  Предполагаю, что поскольку таймеры продолжают работать 
  //        и события захвата генерятся, то это становится причиной прерываний.
  //  Спецификация подобное описывает как "запросы к ядру от запрещенных каналов" - правила DMA 19-21.
  //  Помог запрет обработки одиночных запросов к DMA, выход из прерывания состоялся:
  MDR_DMA->CHNL_USEBURST_SET |= 1 << DMA_CHANNEL_CAP;
  
#else
  
  //  Переинициализация следующего цикла
  BRD_DMA_Write_ChannelCtrl(DMA_CHANNEL_CAP, dmaChanCtrlStart);
  DMA_Cmd(DMA_CHANNEL_CAP, ENABLE);  
#endif  
  
  //  Выставление флага на обработку результатов
  DoCheckResult = 1;

  // Сброс возможных отложенных прерываний
  NVIC_ClearPendingIRQ (DMA_IRQn);
}

void TimerCap_CallDMAEna(FunctionalState enable)
{
#ifdef USE_BOARD_VE_1
  TIMER_DMACmd(brdTimerCap.TIMERx, TIM_DMA_SREQ_CCR, TIMER_DMA_Channel0, enable);
#else
  TIMER_DMACmd(brdTimerCap.TIMERx, TIM_DMA_SREQ_CCR, enable);
#endif
}


//  --- Далее комментировать особо нечего - нажатие кнопков и простейший вариант подсчета периода захвата  ---

void ProcessBtnCommands(void)
{
  if (BRD_Is_BntAct_Up())
  {
    while (BRD_Is_BntAct_Up());
    
    ++capPeriodPWM;
    TIMER_SetCntAutoreload(brdTimerPWM.TIMERx, capPeriodPWM);
    
    LCD_ShowSettings(capPeriodPWM, passDataCap);
  }
  
  if (BRD_Is_BntAct_Down())
  {
    while (BRD_Is_BntAct_Down());
    
    if (capPeriodPWM > 0)
    {  
      --capPeriodPWM;
      TIMER_SetCntAutoreload(brdTimerPWM.TIMERx, capPeriodPWM);
      
      LCD_ShowSettings(capPeriodPWM, passDataCap);
    }  
  }  
  
  if (BRD_Is_BntAct_Left())
  {
    while (BRD_Is_BntAct_Left());
    
    if (passDataCap > 0)
      --passDataCap;
    
    LCD_ShowSettings(capPeriodPWM, passDataCap);
  } 

  if (BRD_Is_BntAct_Right())
  {
    while (BRD_Is_BntAct_Right());
    
    if (passDataCap < TIM_PWM_WIDTH)
      ++passDataCap;
    
    LCD_ShowSettings(capPeriodPWM, passDataCap);
  } 
}

void ClearCaptureData(uintCCR_t data)
{
  uint32_t i;
  for (i = 0; i < DATA_COUNT; ++i)
    arrDataCCR[i] = data;
}

uint32_t CalcPeriod(uint32_t passOffset, uint32_t* errCnt)
{
	uint16_t i;
  uint32_t sum;
	
  *errCnt = 0;
  sum = 0;
  for (i = passOffset; i < (DATA_COUNT - 1); ++i)
  {
    arrPeriod[i] = arrDataCCR[i+1] - arrDataCCR[i];
    sum = sum + arrPeriod[i];
        
    if (arrPeriod[i] == 0)
        ++(*errCnt);
  }
   
  return sum / (DATA_COUNT - passOffset - 1);
}

void LCD_ShowResult(uint32_t periodCap, uint32_t errCnt)
{
  static char message[64];
  
  sprintf(message , "Pc=%d  Err=%d ", periodCap, errCnt);
  BRD_LCD_Print (message, 5);
}

void LCD_ShowSettings(uint32_t periodPWM, uint32_t dataOffs)
{
  static char message[64];
  
  sprintf(message , "Pw=%d  Offs=%d ", periodPWM, dataOffs);
  BRD_LCD_Print (message, 3);
}
