#include "brdClock.h"
#include "brdLed.h"
#include "brdBtn.h"
#include "brdUtils.h"
#include "brdSPI.h"
#include "brdDMA.h"

#include "brdDef.h"
//  Выбор SPI и настроек - модифицируется под реализацию
#include "brdSPI_Select.h"
//  Выбор настроек DMA - модифицируется под реализацию
#include "brdDMA_Select.h"

//  Выбор мастера и ведомого
#define   pSPI_M    pBRD_SPI1   //  Master
#define   pSPI_S    pBRD_SPI2   //  Slave

//  Массивы для передачи по SPI
//    Данных по SPI передаем заведомо больше, чем цикл DMA - чтобы на момент прерывания DMA передача продолжалась.
//    Это хоть каким-то образом эмулирует то, что мастер на самом деле должен быть внешний и слать данные непрерывно.
#define   DATA_COUNT  300
uint16_t  dataTX[DATA_COUNT] __attribute__((section("EXECUTABLE_MEMORY_SECTION"))) __attribute__ ((aligned (4)));
uint16_t  dataRX[DATA_COUNT] __attribute__((section("EXECUTABLE_MEMORY_SECTION"))) __attribute__ ((aligned (4)));

//  Параметры DMA
#define   DMA_CHAN_RX     DMA_Channel_SSP2_RX
#define   DMA_SEND_COUNT  256
uint32_t  dmaChanCtrlStart;

//  Индикация статуса теста. Выводы светодиодов VD6-VD9 заняты под SPI2!
//    Светодиоды гасятся при нажатии на кнопку Select
#define  LED_OK     BRD_LED_5   //  VD10 - загорается при совпадении переданных данных
#define  LED_ERR    BRD_LED_6   //  VD11 - загорается при НЕ совпадении ... 

void PrepareData(void);
void SendDataSPI(void);
uint32_t CountDataErr(void);

//  Флаги
uint32_t DoCheckData;
uint32_t DoSendData;

int main(void)
{
  uint32_t errCount;  
  
  // Максимальное тактирование
  BRD_Clock_Init_HSE_PLL(RST_CLK_CPU_PLLmul16); // 128MHz
  
  //  Инициализация светодиодов и кнопок.
  //  Используется только кнопка Select - гасит светодиоды (стирание текущего статуса)
  BRD_LEDs_InitEx(LED_OK | LED_ERR);
  BRD_BTNs_Init();
  
  //  ----  SPI --- Параметры настройки SPI в brdSPI_Select.h
  //    Скорость обмена максимальная. Для Slave это 128MHz / 12.
  //    SPI Master
  BRD_SPI_PortInit(pSPI_M);
  BRD_SPI_Init(pSPI_M, 1);
  
  //    SPI Slave
  BRD_SPI_PortInit(pSPI_S);
  BRD_SPI_Init(pSPI_S, 0);  
  
  //  ----  DMA --- Параметры настройки DMA в brdDMA_Select.h
  BRD_DMA_Init();
  
  DMA_DataCtrl_Pri.DMA_SourceBaseAddr = (uint32_t)&pSPI_S->SPIx->DR;
  DMA_DataCtrl_Pri.DMA_DestBaseAddr   = (uint32_t)&dataRX;
  DMA_DataCtrl_Pri.DMA_CycleSize      = DMA_SEND_COUNT;
  
  BRD_DMA_Init_Channel(DMA_CHAN_RX, &DMA_ChanCtrl);
  BRD_DMA_InitIRQ(DMA_IRQ_PRIORITY);
  
  //  Управляющее слово для перезапуска цикла DMA
  dmaChanCtrlStart = BRD_DMA_Read_ChannelCtrl(DMA_CHAN_RX);  
  
  //  Разрешение запросов SREQ от SPI к DMA
  SSP_DMACmd(pSPI_S->SPIx, SSP_DMA_RXE, ENABLE);  
  
  //  Подготовка данных и выставление флага для начала передачи pSPI_M -> pSPI_S.
  PrepareData();  
  DoSendData = 1;
  
  //  Выключение индикации статуса
  BRD_LED_Set(LED_OK | LED_ERR, 0);
  
  while (1)
  { 
    //  Посылка данных, посылаем больше чем цикл DMA.
    //  Эмулирует посылку данных от другого источника (МК).
    if (DoSendData)
    {
      SendDataSPI();
      DoSendData = 0;
    }
    else
      //  По факту обработчик DMA_IRQHandler вызывается в процессе SendDataSPI();
      //  Поэтому данная ветвь для подстраховки - чтобы FIFO SPI было не пусто и если возникает
      //  зацикливание прерывания, то это можно было отладить.
      BRD_SPI_FillFIFO_TX(pSPI_M, 0xABCD);
  
    //  Проверка принятых данных. dataTX[] -> pSPI_M -> pSPI_S -> DMA -> dataRX[]
    if (DoCheckData)
    {
      //  Отображение статуса на светодиодах.
      errCount = CountDataErr();
      if (errCount)
        BRD_LED_Set(LED_ERR, 1);
      else
        BRD_LED_Set(LED_OK, 1);

      //  В этот момент данные записанные в FIFO мастера еще передаются.
      //  Ждем пока опустеет FIFO и выставится флаг активности передатчика.
      BRD_SPI_Wait_ClearFIFO_TX(pSPI_M);
      BRD_SPI_Wait_WhileBusy(pSPI_M);
      //  Можно проверить и активность приемника, но в данном примере это никак не сказывается.
      //BRD_SPI_Wait_Busy(pSPI_S);
      
      //  Стираем данные, которые оказались в FIFO slave пока SPI продолжали общение.
      //  Иначе при включении DMA эти данные тут же скопируются из FIFO в dataRX[],
      //  т.е. свежие данные из dataTX попадут в dataRX начиная с 8-го индекса. Размер FIFO = 8.
      BRD_SPI_ClearFIFO_RX(pSPI_S);      
      
      //  Подготавливаем данные для нового цикла
      PrepareData();
      
      //  Включает работу канала DMA      
      DMA_Cmd(DMA_CHAN_RX, ENABLE);
      //  Разрешаем запросы SREQ от SPI к DMA
      SSP_DMACmd(pSPI_S->SPIx, SSP_DMA_RXE, ENABLE);
      
      //  Сбрасываем флаги
      DoCheckData = 0;      
      DoSendData = 1;
    }
    
    //  Поскольку светодиоды статуса при тесте только включаются, 
    //  то для стирания текущего статуса используется кнопке Select.
    if (BRD_Is_BntAct_Select())
    {
      //  Выключение светодиодов
      BRD_LED_Set(LED_OK | LED_ERR, 0);
      
      //  Светодиоды гаснут на время удержания кнопки нажатой
      //  Это показывает, что статус сбрасывается. Иначе можно не заметить что светодиод мигнул.
      while (!BRD_Is_BntAct_Select());
    }
  }
}

void DMA_IRQHandler (void)
{
  //  Запрещаем SSP формировать запросы SREQ к DMA
  SSP_DMACmd(pSPI_S->SPIx, SSP_DMA_RXE, DISABLE);
  //  Восстанавливаем контрольное слово канала для запуска следующего цикла.
  //  Передача не начнется, потому что бит канала в регистре MDR_DMA->CHNL_ENABLE
  //  сброшен аппаратно. Для запуска потребуется выставить этот бит - DMA_Cmd(DMA_CHAN_RX, ENABLE);
  BRD_DMA_Write_ChannelCtrl(DMA_CHAN_RX, dmaChanCtrlStart);  

  //  Флаг на обработку и выход
  DoCheckData = 1;
  NVIC_ClearPendingIRQ (DMA_IRQn);
}	

void SendDataSPI(void)
{
  uint32_t i;
  
  for (i = 0; i < DATA_COUNT; ++i)
  {
    //  Ждем пока в FIFO TX появится место (FIFO Not Full)
    while (SSP_GetFlagStatus(pSPI_M->SPIx, SSP_FLAG_TNF) != SET){};
    //  Записываем данные в FIFO для отправки.
    BRD_SPI_SendValue(pSPI_M, dataTX[i]);
  }
}

void PrepareData(void)
{
  uint16_t i;
  
  for (i = 0; i < DATA_COUNT; ++i)
  {
    //  Перебираем все значения байта, поэтому для теста подойдет индекс
    //  (SPI настроен на обмен 8-битными данными)
    dataTX[i] = i;      
    //  Для определенности данные в массиве забиваются значениями превышающими байт.
    //  SPI FIFO 16-ти битное, поэтому и массивы 16-битные. 
    //  Значения не равные 0xFFFF - это будут те значения, что записал канал DMA.
    dataRX[i] = 0xFFFF;
  }
}

//  Возращаем количество не совпавших при обмене данных
//  В dataRX[] попадут только данные от DMA! 
uint32_t CountDataErr(void)
{
  uint32_t i;
  uint32_t cntErr = 0;  
  
  for (i = 0; i < DMA_SEND_COUNT; ++i)
  {
    if (dataTX[i] != dataRX[i])
      ++cntErr;
  }
  
  return cntErr;
}
