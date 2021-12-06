#ifndef _SECCNT_H_
#define _SECCNT_H_
#include "../common.h"

class secCnt{
private:
	u8					intVecId;
	u32					threshold;
	friend void         secCntIsr(void*);
    void (*callBackHandler)(void *) = NULL;
    void *callBackArg = NULL;
public:
	secCnt(
		XilIntc* 			intc,
		u8 					intVecId,
		u32					threshold,
		u32 				baseAddr,
		void 				(*callBackHandler)(void *),
		void 				*callBackArg
	);
	u32 				baseAddr;

//    void registerCallback(XilIntc* intc, void *(*callBackHandler)(void *), void *callBackArg);
    void setThreshold(u32 threshold);
    u32  readSecCnt();
};


#endif
