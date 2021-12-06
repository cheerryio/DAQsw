#include "TskTop.h"
#include <xparameters.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "queue.h"
#include "../TskGps/TskGps.h"
#include "../TskData/TskData.h"
#include "../TskSD/TskSD.h"

const int tskTopPrio = 7;
const int tskTopStkSize = 8192;
TaskHandle_t 				tskTopHandle;

QueueHandle_t 				tskTopMsgQ;
extern QueueHandle_t 		tskDbgUartMsgQ;

ctrlReg_t 					ctrlReg;
extern u8					isDbgGpsInfo;

extern unsigned int SD_FREE_SIZE;
extern QueueHandle_t tskDataMsgQ;

void testGps()
{
	dbgDateTime(&ctrlReg.curTime);
	dbgPosition(ctrlReg.curAltitude, ctrlReg.curLatitude, ctrlReg.curLongitude);
}

static void tskTop(void *arg)
{
	ctrlReg.If_Start = 0;
    topMode mode;
    mode = Idle;
    tskTopMsg_t msg;
	tskDbgUartMsg_t dgbMsg;
	char dbgStr[64];
	int ret;
	char dirNamePtr[64]="/";
	char fileNamePtr[64]="aaa.txt";
	FIL wf;
	while(1)
	{
		switch(mode){
			case Idle:
				isDbgGpsInfo = 0;
			    if(xQueueReceive(tskTopMsgQ, &msg, portMAX_DELAY))
			    {
			    	processTransSysMsg(&mode, &ctrlReg, msg);
			    }
			    break;
			case Test_GPS:
//				isDbgGpsInfo = 1;
				testGps();
			    if(xQueueReceive(tskTopMsgQ, &msg, 900))
			    {
			    	processTransSysMsg(&mode, &ctrlReg, msg);
			    }
				break;
			case Test_SD:
				SDTest();
				printf("test SD finish\r\n");
				mode=Idle;
				break;
			case Test_Acquisition_Data:
				test_acquisition_data();
				mode=Idle;
				break;
			case Test_Acquisition_FFT:
				test_acquisition_FFT();
				mode=Idle;
				break;
			case Running:
				break;
		};
//		vTaskDelay(100);
	}
}

void processTransSysMsg(topMode* mode, ctrlReg_t* param, tskTopMsg_t msg)
{
	switch(msg.id)
	{
		case ConfigMode:
			*mode = (topMode)msg.data;
			break;
		case ConfigStartYear:
			param->startTime.tm_year = msg.data - 1900;
			break;
		case ConfigStartMonth:
			param->startTime.tm_mon = msg.data - 1;
			break;
		case ConfigStartDay:
			param->startTime.tm_mday = msg.data;
			break;
		case ConfigStartHour:
			param->startTime.tm_hour = msg.data;
			break;
		case ConfigStartMin:
			param->startTime.tm_min = msg.data;
			break;
		case ConfigStartSecond:
			param->startTime.tm_sec = msg.data;
			break;
		case ConfigEndYear:
			param->endTime.tm_year = msg.data - 1900;
			break;
		case ConfigEndMonth:
			param->endTime.tm_mon = msg.data - 1;
			break;
		case ConfigEndDay:
			param->endTime.tm_mday = msg.data;
			break;
		case ConfigEndHour:
			param->endTime.tm_hour = msg.data;
			break;
		case ConfigEndMin:
			param->endTime.tm_min = msg.data;
			break;
		case ConfigEndSecond:
			param->endTime.tm_sec = msg.data;
			break;
		case ConfigSecondsIncrease:
			param->secondsIncrease=msg.data;
			break;
		case ConfigIfFft:
			param->If_FFT = msg.data;
			break;
		case SetStart:
			param->If_Start = msg.data;
			break;
	}
}

void tskTopInit()
{
	int ret;
	tskTopMsgQ = xQueueCreate(30, sizeof(tskTopMsg_t));
	configASSERT(tskTopMsgQ);

    ret = xTaskCreate(tskTop, "TskTop", tskTopStkSize, NULL, tskTopPrio, &tskTopHandle);
    configASSERT(ret == pdPASS);
}
