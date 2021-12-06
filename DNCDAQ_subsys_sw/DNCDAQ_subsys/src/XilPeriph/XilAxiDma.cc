#include "XilAxiDma.h"

XilAxiDma::XilAxiDma
(
		u8				deviceId,
		u8				mm2sIntVecId,
		u8				s2mmIntVecId,
		void*			axiDmaMM2SIsr(void*),
		void*			axiDmaS2MMIsr(void*)
)
{
	int Status;
	this->deviceId = deviceId;
	this->mm2sIntVecId = mm2sIntVecId;
	this->s2mmIntVecId = s2mmIntVecId;
	XilAxiDmaInit();
}

int XilAxiDma::XilAxiDmaInit()
{
	int status;

	XAxiDma_Config 			*Config = NULL;
	Config = XAxiDma_LookupConfig(this->deviceId);
	if(Config == NULL){
		xil_printf("No config found for DMA %d\r\n", deviceId);
		return XST_FAILURE;
	}

	status = XAxiDma_CfgInitialize(&(this->Dma), Config);		//lost 4 data
	if(status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	return 0;
}

int XilAxiDma::DmaSimpleTransfer(UINTPTR BuffAddr, u32 Length, int Direction)
{
	int Status;

	Status =  XAxiDma_SimpleTransfer(&(this->Dma), BuffAddr, Length, Direction);
	return Status;
}

void XilAxiDma::DmaReset(){
	XAxiDma_Reset(&(this->Dma));
	while(!XAxiDma_ResetIsDone(&(this->Dma)));
}

XAxiDma* XilAxiDma::returnXAxiDma()
{
	return &this->Dma;
}
