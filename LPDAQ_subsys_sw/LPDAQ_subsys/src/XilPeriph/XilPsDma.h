#ifndef __XIL_PS_DMA_H__
#define __XIL_PS_DMA_H__
#include "xdmaps.h"
#include "xil_printf.h"
class XilPsDma{
private:
	u8						deviceId;
	XDmaPs 					Dma;
	XDmaPs_Cmd				dmaps_cmd;
public:
	XilPsDma(
		u8				deviceId
	);

	int XilPsDmaInit();
	int DmaSimpleTransfer(
		unsigned int SrcBurstSize,
		unsigned int SrcBurstLen,
		unsigned int SrcInc,
		unsigned int DstBurstSize,
		unsigned int DstBurstLen,
		unsigned int DstInc,
		u32 SrcAddr,
		u32 DstAddr,
		unsigned int Length
		);
	void DmaReset();
	XDmaPs* returnPsDma();
};

#endif
