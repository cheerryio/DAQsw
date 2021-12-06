#ifndef _XILUARTPS_H_
#define _XILUARTPS_H_
#include "xuartps.h"
#include "XilIntc.h"
#include "xil_printf.h"


class XillUartps{
private:
	u8					deviceId;
	XUartPs				uart;
	XUartPs_Config*		uartCfg;
	u32					baudRate;
	u8					intVecId;
public:
	XillUartps(
		u8 deviceId,
		u32	baudRate,
		XilIntc* intc,
		u8 intVecId,
		void (*uartIsr)(void *, u32 , u32)
	);

	int XilUartpsInit();
	int XilUartpsRegisterInterrupt(XilIntc* intc, void (*uartIsr)(void *, u32 , u32));
	u32 XilUartPs_Send(u8 *BufferPtr, u32 NumBytes);

	u32 XilUartPs_Recv(u8 *BufferPtr, u32 NumBytes);

	void XilUartpsSetFifoThreshold(u8 threshold);
	void XilUartpsResetHw();
	void XilUartpsSetIntMask(u32 mask);
	XUartPs* returnXUartPs();
};


#endif
