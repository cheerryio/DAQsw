#include "XilUartps.h"

//
//void XUartPs_SetHandler(XUartPs *InstancePtr, XUartPs_Handler FuncPtr,
//			 void *CallBackRef);

XUartPsFormat uart_format =
{
	//9600,
	XUARTPS_DFT_BAUDRATE,   //默认波特率 115200
	XUARTPS_FORMAT_8_BITS,
	XUARTPS_FORMAT_NO_PARITY,
	XUARTPS_FORMAT_1_STOP_BIT,
};


XillUartps::XillUartps(
		u8 deviceId,
		u32	baudRate,
		XilIntc* intc,
		u8 intVecId,
		void (*uartIsr)(void *, u32 , u32)
)
{
	this->deviceId = deviceId;
	this->baudRate = baudRate;
	this->intVecId = intVecId;

	if(
			intc	!= NULL		&&
			uartIsr != NULL
	)
	{
		XilUartpsInit();
		XilUartpsRegisterInterrupt(intc, uartIsr);
	}
	else
	{
		XilUartpsInit();
	}

}

int XillUartps::XilUartpsInit()
{
	int status;
	uartCfg = XUartPs_LookupConfig(deviceId);

	if(uartCfg == NULL){
		return XST_FAILURE;
	}

	XUartPs_ResetHw(uartCfg->BaseAddress);
	Xil_Out32((uartCfg->BaseAddress + (u32)XUARTPS_CR_OFFSET), \
		  (((Xil_In32(uartCfg->BaseAddress + (u32)XUARTPS_CR_OFFSET)) & \
		  (u32)(~XUARTPS_CR_EN_DIS_MASK)) | ((u32)XUARTPS_CR_RX_DIS | (u32)XUARTPS_CR_TX_DIS)));		// Diable Uart
	status = XUartPs_CfgInitialize(&uart, uartCfg, uartCfg->BaseAddress);
	if(status != XST_SUCCESS){
		return XST_FAILURE;
	}

	XUartPs_SetOperMode(&uart, XUARTPS_OPER_MODE_NORMAL);

	status = XUartPs_SetBaudRate(&uart, baudRate);
	if(status != XST_SUCCESS){
		return XST_FAILURE;
	}
	XUartPs_EnableUart(&uart);
	return XST_SUCCESS;
}
int XillUartps::XilUartpsRegisterInterrupt(XilIntc* intc, void (*uartIsr)(void *, u32 , u32))
{
	int status;

	XUartPs_SetHandler(&uart, (XUartPs_Handler)uartIsr, (void *)this);
	status = intc->InitChannelInt(intVecId, (XInterruptHandler) XUartPs_InterruptHandler, (void *)&(this->uart));
	if(status != XST_SUCCESS){
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}
u32 XillUartps::XilUartPs_Send(u8 *BufferPtr, u32 NumBytes)
{
	return XUartPs_Send(&uart, BufferPtr, NumBytes);
}

u32 XillUartps::XilUartPs_Recv(u8 *BufferPtr, u32 NumBytes)
{
	return XUartPs_Recv(&uart, BufferPtr, NumBytes);
}

void XillUartps::XilUartpsSetFifoThreshold(u8 threshold)
{
	XUartPs_SetFifoThreshold(&uart, threshold);
}

void XillUartps::XilUartpsSetIntMask(u32 mask)
{
	XUartPs_SetInterruptMask(&uart, mask);
}

void XillUartps::XilUartpsResetHw()
{
	XUartPs_ResetHw(uartCfg->BaseAddress);
}

XUartPs* XillUartps::returnXUartPs()
{
	return &this->uart;
}
