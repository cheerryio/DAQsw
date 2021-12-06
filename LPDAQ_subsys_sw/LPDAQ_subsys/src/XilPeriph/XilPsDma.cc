#include "XilPsDma.h"

XilPsDma::XilPsDma
(
		u8				deviceId
)
{
	int Status;
	this->deviceId = deviceId;
	XilPsDmaInit();
}

int XilPsDma::XilPsDmaInit()
{
	int status;

	XDmaPs_Config *Config = NULL;
	Config = XDmaPs_LookupConfig(this->deviceId);
	if(Config == NULL){
		xil_printf("No config found for DMA %d\r\n", deviceId);
		return XST_FAILURE;
	}

	status = XDmaPs_CfgInitialize(&(this->Dma),Config,Config->BaseAddress);		//lost 4 data
	if(status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	return 0;
}

int XilPsDma::DmaSimpleTransfer(
		unsigned int SrcBurstSize,
		unsigned int SrcBurstLen,
		unsigned int SrcInc,
		unsigned int DstBurstSize,
		unsigned int DstBurstLen,
		unsigned int DstInc,
		u32 SrcAddr,
		u32 DstAddr,
		unsigned int Length
	)
{
	int status;
	memset(&(this->dmaps_cmd),0,sizeof(XDmaPs_Cmd));
	this->dmaps_cmd.ChanCtrl.SrcBurstSize=SrcBurstSize;
	this->dmaps_cmd.ChanCtrl.SrcBurstLen=SrcBurstLen;
	this->dmaps_cmd.ChanCtrl.SrcInc=SrcInc;
	this->dmaps_cmd.ChanCtrl.DstBurstSize=DstBurstSize;
	this->dmaps_cmd.ChanCtrl.DstBurstLen=DstBurstLen;
	this->dmaps_cmd.ChanCtrl.DstInc=DstInc;
	this->dmaps_cmd.BD.SrcAddr=SrcAddr;
	this->dmaps_cmd.BD.DstAddr=DstAddr;
	this->dmaps_cmd.BD.Length=Length;
	status=XDmaPs_Start(&(this->Dma),0,&(this->dmaps_cmd),0);
	return status;
}

void XilPsDma::DmaReset(){
	XDmaPs_ResetManager(&(this->Dma));
	XDmaPs_ResetChannel(&(this->Dma),0);
}

XDmaPs* XilPsDma::returnPsDma()
{
	return &(this->Dma);
}
