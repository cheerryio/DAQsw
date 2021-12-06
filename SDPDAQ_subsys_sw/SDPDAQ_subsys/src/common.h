#ifndef _COMMON_H_
#define _COMMON_H_
#include "xparameters.h"
#include "XilPeriph/XilIntc.h"
#include "XilPeriph/XilUartps.h"
#include "XilPeriph/XilGpio.h"
#include "XilPeriph/XilAxiDma.h"
#include "TskData/secCnt.h"
#include "time.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
void initPeriph();

#define DBG_INFO 0

const unsigned int MAX_MODE_NUM = 6;
typedef enum topMode
{
	Idle = 0,
	Test_GPS,
	Test_SD,
	Test_Acquisition_Data,
	Test_Acquisition_FFT,
	Running
};

enum tskTopMsgID_t : u32{
	ConfigMode,
	ConfigStartYear,
	ConfigStartMonth,
	ConfigStartDay,
	ConfigStartHour,
	ConfigStartMin,
	ConfigStartSecond,

	ConfigEndYear,
	ConfigEndMonth,
	ConfigEndDay,
	ConfigEndHour,
	ConfigEndMin,
	ConfigEndSecond,

	ConfigSecondsIncrease,
	ConfigIfFft,
	SetStart
};

typedef struct tskTopMsg_t{
	tskTopMsgID_t			id;
	u32						data;
};

typedef struct tskDbgUartMsg_t{
//	tskTransUartMsgID_t		id;
	u32						address;
	u32						size;
};

enum tskDataID_t:u32{
	doData,
	dofft
};

typedef struct tskDataMsg_t{
	tskDataID_t id;
};

enum tskWriteID_t : u32{
	WriteOneSec,
	WriteFirstSec,
	WriteLastSec,
	WriteChangeFile
};

typedef struct tskWriteMsg_t{
	tskWriteID_t id;
};

typedef struct dtime{
	tm						dateTime;
	u16						millis;
};

typedef struct ctrlReg_t{
	u8						If_Start;
	u8						dataReading;
	u8						If_FFT;
	u8 						adcTesting;
	u8						fftTesting;
	tm						startTime;
	tm						endTime;
	tm						curTime;
	float 					curLatitude,
							curLongitude,
							curAltitude;
	unsigned int			secondsIncrease=30;
};

typedef struct FuncAndCmd
{
    void (*func)(char *cmd);
    const char *cmd;
};

#endif

