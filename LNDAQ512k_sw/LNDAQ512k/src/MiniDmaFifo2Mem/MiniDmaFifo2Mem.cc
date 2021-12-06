/*
 * MiniDma.cc
 *
 *  Created on: Jun 18, 2018
 *      Author: loywong
 */

#include "xil_types.h"
#include "xil_io.h"
#include "MiniDmaFifo2Mem.h"

void miniDmaFifo2MemIsr(void *arg)
{
	MiniDmaFifo2Mem *me = (MiniDmaFifo2Mem *)arg;
	MiniDmaFifo2Mem::RegIntr intSts;
	// read int status
	intSts.Val = Xil_In32(me->b + me->regIntSts);
	// clear int statue
	Xil_Out32(me->b + me->regIntSts, intSts.Val);
	// acknowledge_interrupt(intId);
	XIntc_Acknowledge(me->intc, me->intId);
	// call callback func
	u64 prevAddr = Xil_In64(me->b + me->regPrevAddr);
	me->cb(intSts, prevAddr, me->cbArg);
}

MiniDmaFifo2Mem::MiniDmaFifo2Mem(u32 base) {
	this->b = base;
}

MiniDmaFifo2Mem::~MiniDmaFifo2Mem() {
	// TODO Auto-generated destructor stub
}

void MiniDmaFifo2Mem::RegisterCallback(XIntc *intc, unsigned int intId, void (*cb)(RegIntr, u64, void *), void *arg)
{
	this->intc = intc;
	this->intId = intId;
	this->cb = cb;
	this->cbArg = arg;
	if(cb != NULL)
	{
		u32 sts;

        sts = XIntc_Connect(intc, intId, miniDmaFifo2MemIsr, (void *)this);
//      sts = register_int_handler(intId, miniDmaFifo2MemIsr, (void *)this);
        if(sts != XST_SUCCESS)
        {
            throw "Register isr in HwFifo::HwFifo error!\r\n";
        }
//      acknowledge_interrupt(intId);
        XIntc_Acknowledge(intc, intId);
//      enable_interrupt(intId);
        XIntc_Enable(intc, intId);
        // ie
        Xil_Out32(this->b + regIntEn, 0x3);	// enable both(error & finish)
	}
}

void MiniDmaFifo2Mem::SetAddress(u64 addr0, u64 addr1)
{
	Xil_Out64(this->b + this->regAddr0, addr0);
	Xil_Out64(this->b + this->regAddr1, addr1);
}

void MiniDmaFifo2Mem::SetFifoPentTh(u32 pth)
{
	Xil_Out32(this->b + this->regFifoPentTh, pth);
}
void MiniDmaFifo2Mem::SetControl(RegControll ctrl)
{
	Xil_Out32(this->b + this->regControl, ctrl.Val);
}

MiniDmaFifo2Mem::RegStatus MiniDmaFifo2Mem::GetStatus()
{
	RegStatus sts;
	Xil_In32(this->b + this->regStatus);
	return sts;
}

void MiniDmaFifo2Mem::SetTransferLen(u32 len)
{
	if(len)
	{
		Xil_Out32(this->b + this->regTransLen, len - 1);	// set trans len
	}
}

void MiniDmaFifo2Mem::StartTransfer(u32 len)
{
	if(len)
	{
		Xil_Out32(this->b + this->regTransLen, len - 1);	// set trans len
	}
	Xil_Out32(this->b + this->regStart, 1);			// start
}
