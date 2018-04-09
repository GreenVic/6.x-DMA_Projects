#ifndef _BRD_DMA_H
#define _BRD_DMA_H

#include <MDR32F9Qx_dma.h>

void BRD_DMA_Init(void);
void BRD_DMA_Init_Channel (uint32_t DMA_Channel, DMA_ChannelInitTypeDef*  pDMAChannelInitStruct);

//  For fast reinit DMA Channel - save before DMA start and restore on DMA_IRQ
void BRD_DMA_Read_ChannelCtrl(uint32_t DMA_Channel, uint32_t *dmaControl);
void BRD_DMA_Write_ChannelCtrl(uint32_t DMA_Channel, uint32_t dmaControl);

//  Uncomment if needed
//void BRD_DMA_Read_ChannelCtrlRec(uint32_t DMA_Channel, DMA_CtrlDataTypeDef *chanTable);
//void BRD_DMA_Write_ChannelCtrlRec(uint32_t DMA_Channel, uint32_t *chanTable);

#endif	// _BRD_DMA_H
