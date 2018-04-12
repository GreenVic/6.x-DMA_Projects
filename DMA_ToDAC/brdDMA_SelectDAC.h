#ifndef _BRD_DMA_SELECT_DAC_H
#define _BRD_DMA_SELECT_DAC_H

#include "brdDMA.h"

// Настройки управления данными DMA
DMA_CtrlDataInitTypeDef DMA_DataCtrl_Pri = 
{
  0,                            // DMA_SourceBaseAddr - Адрес источника данных
  0,                            // DMA_DestBaseAddr   - Адрес назначения данных
  DMA_SourceIncHalfword,        // DMA_SourceIncSize  - Автоувеличение адреса источника данных
  DMA_DestIncNo,                // DMA_DestIncSize    - Автоувеличение адреса назначения данных
  DMA_MemoryDataSize_HalfWord,  // DMA_MemoryDataSize - Размер пакета данных
  DMA_Mode_Basic,               // DMA_Mode           - Режим работы DMA
  10,                           // DMA_CycleSize      - Кол. данных на передачу (длина цикла DMA)
  DMA_Transfers_1,              // DMA_NumContinuous  - Количество непрерывных передач (до арбитража)
                                //   В 1986ВЕ1Т другие значения не использовать  - таймер вырабатывает req - DMA выводит весь период за один запрос таймера (CNT ==ARR)!
                                //   В 1986ВЕ9х можно вплоть до DMA_Transfers_1  - таймер НЕ вырабатывает req (только sreq), DMA выводит по одному значению за один запрос таймера (CNT ==ARR)!
  DMA_SourcePrivileged,         // DMA_SourceProtCtrl - Режим защиты передатчика
  DMA_DestPrivileged            // DMA_DestProtCtrl   - Режим защиты приемника
};
  
//  Настройки канала DMA
DMA_ChannelInitTypeDef DMA_ChanCtrl = 
{
  &DMA_DataCtrl_Pri,        // DMA_PriCtrlData         - Основная структура управления данными
  &DMA_DataCtrl_Pri,        // DMA_AltCtrlStr          - Альтернативная структура управления данными
   0, //DMA_AHB_Privileged,      // DMA_ProtCtrl 
   DMA_Priority_Default,    // DMA_Priority            - Приоритет канала
   DMA_BurstClear,          // DMA_UseBurst
   DMA_CTRL_DATA_PRIMARY    // DMA_SelectDataStructure - Используемая структура управления данными
};
  
#endif	// _BRD_DMA_SELECT_DAC_H


