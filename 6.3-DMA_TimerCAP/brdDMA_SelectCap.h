#ifndef _BRD_DMA_SELECT_H
#define _BRD_DMA_SELECT_H

#include "brdDMA.h"

// ��������� ���������� ������� DMA
DMA_CtrlDataInitTypeDef DMA_DataCtrl_Pri = 
{
  .DMA_SourceBaseAddr = 0,                            // DMA_SourceBaseAddr - ����� ��������� ������
  .DMA_DestBaseAddr   = 0,                            // DMA_DestBaseAddr   - ����� ���������� ������
  .DMA_SourceIncSize  = DMA_SourceIncNo,              // DMA_SourceIncSize  - �������������� ������ ��������� ������
  .DMA_DestIncSize    = DMA_DestIncHalfword,          // DMA_DestIncSize    - �������������� ������ ���������� ������
  .DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,  // DMA_MemoryDataSize - ������ ������ ������
  .DMA_Mode           = DMA_Mode_Basic,               // DMA_Mode           - ����� ������ DMA
  .DMA_CycleSize      = 10,                           // DMA_CycleSize      - ���. ������ �� �������� (����� ����� DMA)
  .DMA_NumContinuous  = DMA_Transfers_1,              // DMA_NumContinuous  - ���������� ����������� ������� (�� ���������)
  .DMA_SourceProtCtrl = DMA_SourcePrivileged,         // DMA_SourceProtCtrl - ����� ������ �����������
  .DMA_DestProtCtrl   = DMA_DestPrivileged            // DMA_DestProtCtrl   - ����� ������ ���������
};
  
//  ��������� ������ DMA
DMA_ChannelInitTypeDef DMA_ChanCtrl = 
{
  .DMA_PriCtrlData = &DMA_DataCtrl_Pri,               // DMA_PriCtrlData    - �������� ��������� ���������� �������
  .DMA_AltCtrlData = &DMA_DataCtrl_Pri,               // DMA_AltCtrlData    - �������������� ��������� ���������� �������
  .DMA_ProtCtrl    = DMA_AHB_Privileged,              // DMA_ProtCtrl 
  .DMA_Priority    = DMA_Priority_Default,            // DMA_Priority       - ��������� ������
  .DMA_UseBurst    = DMA_BurstClear,                  // DMA_UseBurst
  .DMA_SelectDataStructure = DMA_CTRL_DATA_PRIMARY    // DMA_SelectDataStructure - ������������ ��������� ���������� �������
};

#endif	// _BRD_DMA_SELECT_H


