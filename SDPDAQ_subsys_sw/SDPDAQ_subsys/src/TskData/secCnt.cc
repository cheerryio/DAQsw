#include "secCnt.h"


void secCntIsr(void *arg)
{
    secCnt *me = (secCnt *)arg;
    unsigned int msk = Xil_In32(me->baseAddr);
    if(msk & 0x1)
    {
    	Xil_Out32(me->baseAddr, 0);
    	if(NULL != me->callBackHandler)
    		me->callBackHandler(me->callBackArg);
    }

}

secCnt::secCnt(
	XilIntc* 			intc,
	u8 					intVecId,
	u32					threshold,
	u32 				baseAddr,
	void 				(*callBackHandler)(void *),
	void 				*callBackArg
)
{
	this->intVecId = intVecId;
	this->threshold = threshold;
	this->baseAddr = baseAddr;
	this->callBackArg = callBackArg;
	this->callBackHandler = callBackHandler;

	setThreshold(this->threshold);
	int status;
	Xil_Out32(baseAddr, 0);
	status = intc->InitChannelInt(this->intVecId, (XInterruptHandler)secCntIsr, (void *)this);
}

void secCnt::setThreshold(u32 threshold)
{
	Xil_Out32(baseAddr+4, threshold);
}

u32  secCnt::readSecCnt()
{
	return Xil_In32(baseAddr+8);
}
