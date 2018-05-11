#ifndef _BRD_DMA_SELECT_H
#define _BRD_DMA_SELECT_H

#include "brdDMA.h"

// Настройки управления данными DMA
DMA_CtrlDataInitTypeDef DMA_DataCtrl_Pri = 
{
  .DMA_SourceBaseAddr = 0,                            // DMA_SourceBaseAddr - Адрес источника данных
  .DMA_DestBaseAddr   = 0,                            // DMA_DestBaseAddr   - Адрес назначения данных
  .DMA_SourceIncSize  = DMA_SourceIncNo,              // DMA_SourceIncSize  - Автоувеличение адреса источника данных
  .DMA_DestIncSize    = DMA_DestIncHalfword,          // DMA_DestIncSize    - Автоувеличение адреса назначения данных
  .DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,  // DMA_MemoryDataSize - Размер пакета данных
  .DMA_Mode           = DMA_Mode_Basic,               // DMA_Mode           - Режим работы DMA
  .DMA_CycleSize      = 10,                           // DMA_CycleSize      - Кол. данных на передачу (длина цикла DMA)
  .DMA_NumContinuous  = DMA_Transfers_1,              // DMA_NumContinuous  - Количество непрерывных передач (до арбитража)
  .DMA_SourceProtCtrl = DMA_SourcePrivileged,         // DMA_SourceProtCtrl - Режим защиты передатчика
  .DMA_DestProtCtrl   = DMA_DestPrivileged            // DMA_DestProtCtrl   - Режим защиты приемника
};
  
//  Настройки канала DMA
DMA_ChannelInitTypeDef DMA_ChanCtrl = 
{
  .DMA_PriCtrlData = &DMA_DataCtrl_Pri,               // DMA_PriCtrlData    - Основная структура управления данными
  .DMA_AltCtrlData = &DMA_DataCtrl_Pri,               // DMA_AltCtrlData    - Альтернативная структура управления данными
  .DMA_ProtCtrl    = DMA_AHB_Privileged,              // DMA_ProtCtrl 
  .DMA_Priority    = DMA_Priority_Default,            // DMA_Priority       - Приоритет канала
  .DMA_UseBurst    = DMA_BurstClear,                  // DMA_UseBurst
  .DMA_SelectDataStructure = DMA_CTRL_DATA_PRIMARY    // DMA_SelectDataStructure - Используемая структура управления данными
};

#endif	// _BRD_DMA_SELECT_H


