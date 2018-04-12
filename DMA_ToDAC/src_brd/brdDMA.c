#include <MDR32F9Qx_port.h>
#include <MDR32F9Qx_rst_clk.h>

#include "brdDMA.h"
#include "brdDef.h"

void BRD_DMA_Init(void)
{
  // Включение тактирования модуля DMA
  //RST_CLK_PCLKcmd (BRD_DMA_CLOCK_SELECT, ENABLE); - Отключено для теста ошибки DMA при отсутствии тактирования от SSP1, SSP2

  // Деинициализация DMA
  DMA_DeInit();	
  
  // Сброс прерывания от DMA
  NVIC_ClearPendingIRQ (DMA_IRQn);  
}  

// Инициализация модуля DMA
void BRD_DMA_Init_Channel (uint32_t DMA_Channel, DMA_ChannelInitTypeDef*  pDMAChannelInitStruct)
{
  // Инициализация канала DMA,
	DMA_Init(DMA_Channel, pDMAChannelInitStruct);

  // Назначение приоритета аппаратных прерываний от DMA
  NVIC_SetPriority (DMA_IRQn, 1);

  // Включение аппаратных прерываний от DMA
  NVIC_EnableIRQ (DMA_IRQn); 
	
  DMA_Cmd(DMA_Channel, ENABLE);
}

//// Прототип - Обработчик прерываний от DMA
//void DMA_IRQHandler (void)
//{
//  // Формирование нового цикла аналого-цифровых преобразований
//  DMA_Init (DMA_Channel, &DMAChannelInitStruct);
//  NVIC_ClearPendingIRQ (DMA_IRQn);
//}	

extern DMA_CtrlDataTypeDef DMA_ControlTable[DMA_Channels_Number * (1 + DMA_AlternateData)];

void BRD_DMA_Read_ChannelCtrl(uint32_t DMA_Channel, uint32_t *dmaControl)
{
	*dmaControl = DMA_ControlTable[DMA_Channel].DMA_Control;
}

void BRD_DMA_Write_ChannelCtrl(uint32_t DMA_Channel, uint32_t dmaControl)
{
	DMA_ControlTable[DMA_Channel].DMA_Control = dmaControl;
}

//void BRD_DMA_Read_ChannelCtrlRec(uint32_t DMA_Channel, DMA_CtrlDataTypeDef *chanTable)
//{
//	chanTable->DMA_SourceEndAddr = DMA_ControlTable[DMA_Channel].DMA_SourceEndAddr;
//	chanTable->DMA_DestEndAddr   = DMA_ControlTable[DMA_Channel].DMA_DestEndAddr;
//	chanTable->DMA_Control       = DMA_ControlTable[DMA_Channel].DMA_Control;
//}

//void BRD_DMA_Write_ChannelCtrlRec(uint32_t DMA_Channel, uint32_t *chanTable)
//{
//	DMA_ControlTable[DMA_Channel].DMA_SourceEndAddr = chanTable->DMA_SourceEndAddr;
//	DMA_ControlTable[DMA_Channel].DMA_DestEndAddr   = chanTable->DMA_DestEndAddr;
//	DMA_ControlTable[DMA_Channel].DMA_Control       = chanTable->DMA_Control;
//}



