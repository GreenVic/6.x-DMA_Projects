#ifndef _BRD_TASKDEF_H
#define _BRD_TASKDEF_H

#include "brdTimer.h"
#include "brdDMA.h"

//  Pattern for Capture timer pin
PORT_InitTypeDef pinTimerCAP =
{
  .PORT_Pin       = 0,
  .PORT_OE        = PORT_OE_IN,
  .PORT_PULL_UP   = PORT_PULL_UP_OFF,
  .PORT_PULL_DOWN = PORT_PULL_DOWN_ON, //PORT_PULL_DOWN_OFF,
  .PORT_PD_SHM    = PORT_PD_SHM_OFF,
  .PORT_PD        = PORT_PD_DRIVER,
  .PORT_GFEN      = PORT_GFEN_OFF,
  .PORT_FUNC      = PORT_FUNC_MAIN,
  .PORT_SPEED     = PORT_SPEED_MAXFAST,
  .PORT_MODE      = PORT_MODE_DIGITAL
};

PORT_InitTypeDef pinTimerPWM =
{
  .PORT_Pin       = 0,
  .PORT_OE        = PORT_OE_OUT,
  .PORT_PULL_UP   = PORT_PULL_UP_OFF,
  .PORT_PULL_DOWN = PORT_PULL_DOWN_OFF,
  .PORT_PD_SHM    = PORT_PD_SHM_OFF,
  .PORT_PD        = PORT_PD_DRIVER,
  .PORT_GFEN      = PORT_GFEN_OFF,
  .PORT_FUNC      = PORT_FUNC_MAIN,
  .PORT_SPEED     = PORT_SPEED_MAXFAST,
  .PORT_MODE      = PORT_MODE_DIGITAL
};

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

#endif //_BRD_TASKDEF_H

