#include "XilIntc.h"


XilIntc::XilIntc(u8 IntcId)
{
	deviceId = IntcId;
	XilIntcInit();
}

int XilIntc::XilIntcInit()
{
	int Status;

	// Initialize the interrupt controller driver so that it is ready to use.
	GicCfg = XScuGic_LookupConfig(deviceId);
	if (NULL == GicCfg) {
		return XST_FAILURE;
	}
	Status = XScuGic_CfgInitialize(&Intc, GicCfg, GicCfg->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Initialize the exception table.
	Xil_ExceptionInit();

	// interrupt handling logic in the processor.
//	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
//				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
//				&Intc);

	// Enable exceptions.
//	Xil_ExceptionEnable();
	return XST_SUCCESS;
}

int XilIntc::InitChannelInt(u8 InterruptVectorId,XInterruptHandler Interrupt_ISR,void *CallBack)
{
//	XScuGic_InterruptMaptoCpu(intc, XPAR_CPU_ID, intId);
	int Status;
	XScuGic_InterruptMaptoCpu(&Intc, XPAR_CPU_ID, InterruptVectorId);
	Status = XScuGic_Connect(&Intc, InterruptVectorId,
				(XInterruptHandler) Interrupt_ISR,
				(void *) CallBack);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable IO expander and sensor IIC interrupts */
	XScuGic_Enable(&Intc, InterruptVectorId);
	return XST_SUCCESS;
}
void XilIntc::DisableIntrSystem(u8 InterruptVectorId)
{
	XScuGic_Disable(&Intc, InterruptVectorId);
}

XScuGic* XilIntc::returnXIntc()
{
	return &this->Intc;
}

void XilIntc::XilIntcExceptionEnable()
{
//	 Enable exceptions.
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				&Intc);
	Xil_ExceptionEnable();
}
