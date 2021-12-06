/*
 * MiniDma.h
 *
 *  Created on: Jun 18, 2018
 *      Author: loywong
 */

#ifndef SRC_MINIDMAFIFO2MEM_MINIDMAFIFO2MEM_H_
#define SRC_MINIDMAFIFO2MEM_MINIDMAFIFO2MEM_H_

#include "xil_types.h"
#include "xintc.h"

class MiniDmaFifo2Mem {
public:
    union RegControll
	{
    	u32 Val = 0;
    	struct
		{
    		bool Penetrate : 1;
    		bool FrameTrig : 1;
    		bool ContTrig  : 1;
		};
	};
    union RegStatus
    {
    	u32 Val = 0;
    	struct
		{
    		bool Busy		: 1;
    		u32  Bresp		: 2;
    		u32  CurrAddr 	: 1;
		};
    };
    union RegIntr
    {
    	u32 Val = 0;
    	struct
		{
    		bool finish : 1;
    		bool error  : 1;
		};
    };
private:
	const u32 regAddr0	 	= 0x00;
	const u32 regAddr1 		= 0x08;
	const u32 regPrevAddr 	= 0x18;
	const u32 regStart		= 0x20;
	const u32 regTransLen	= 0x24;
	const u32 regFifoPentTh = 0x28;
	const u32 regControl	= 0x2c;
	const u32 regStatus		= 0x30;
	const u32 regIntEn		= 0x34;
	const u32 regIntSts		= 0x38;
	friend void miniDmaFifo2MemIsr(void *arg);
    u32 b;
    XIntc *intc = NULL;
    u32 intId = -1;
    void (*cb)(RegIntr, u64, void *) = NULL;
    void *cbArg = NULL;
public:
	MiniDmaFifo2Mem(u32 base);
	virtual ~MiniDmaFifo2Mem();
	void RegisterCallback(XIntc *intc, unsigned int intId, void (*cb)(RegIntr, u64, void *), void *arg);
	void SetAddress(u64 addr0, u64 addr1);
	void SetFifoPentTh(u32 pth);
	void SetControl(RegControll ctrl);
	RegStatus GetStatus();
	void SetTransferLen(u32 len);
	void StartTransfer(u32 len = 0);
};

#endif /* SRC_MINIDMAFIFO2MEM_MINIDMAFIFO2MEM_H_ */
