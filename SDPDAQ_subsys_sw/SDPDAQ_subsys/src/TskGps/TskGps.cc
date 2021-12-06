#include "TskGps.h"

char 						dbgInfo[100];
char 						dateTimeStr[100];
char 						postionStr[100];
char 						gpsInfo[512];
char 						millisStr[20];
const int tskGpsPrio = 8;
const int tskGpsStkSize = 524288;
static TaskHandle_t 		tskGpsHandle;

extern QueueHandle_t 		tskDbgUartMsgQ;
extern QueueHandle_t 		tskTopMsgQ;


extern XillUartps*			uartGps;
extern XilIntc* 			intc;

SemaphoreHandle_t 			SemRecvGps;

extern ctrlReg_t 			ctrlReg;
tskDbgUartMsg_t 			dbgMsgGpsInfo;
tskDbgUartMsg_t 			dbgMsgDateTime;
tskDbgUartMsg_t 			dbgMsgMillis;
tskDbgUartMsg_t 			dbgMsgPosition;

u8							isDbgGpsInfo;
char gpsGetChar()
{
	u8 c;
	u32 CsrRegister;
    XUartPs* InstancePtr = uartGps->returnXUartPs();

	CsrRegister = XUartPs_ReadReg(InstancePtr->Config.BaseAddress,
				XUARTPS_SR_OFFSET);
	while(CsrRegister & XUARTPS_SR_RXEMPTY)
	{
		vTaskDelay(10);
		CsrRegister = XUartPs_ReadReg(InstancePtr->Config.BaseAddress,
					XUARTPS_SR_OFFSET);
	}
	uartGps->XilUartPs_Recv(&c, 1);
	return (char)c;
}

int checkUart()
{
	u32 CsrRegister;
    XUartPs* InstancePtr = uartGps->returnXUartPs();
	CsrRegister = XUartPs_ReadReg(InstancePtr->Config.BaseAddress,
				XUARTPS_SR_OFFSET);
	if(CsrRegister & XUARTPS_SR_RXFULL)																		// RX Fifo is full
	{
		XUartPs_WriteReg(InstancePtr->Config.BaseAddress, XUARTPS_CR_OFFSET, (u32)XUARTPS_CR_RX_DIS);		//Disable Rx
		XUartPs_WriteReg(InstancePtr->Config.BaseAddress, XUARTPS_CR_OFFSET, ((u32)XUARTPS_CR_RXRST));		//Clear Rx Fifo
		XUartPs_WriteReg(InstancePtr->Config.BaseAddress, XUARTPS_CR_OFFSET, (u32)XUARTPS_CR_RX_EN);		//Enable Rx
		return 1;
	}
	return 0;
}

tm getTm(Ublox m8t){
	tm temp;
	temp.tm_year = m8t.datetime.year - 1900 + 2000;
	temp.tm_mon = m8t.datetime.month - 1;
	temp.tm_mday = m8t.datetime.day;
	temp.tm_hour = m8t.datetime.hours+8;
	temp.tm_min = m8t.datetime.minutes;
	temp.tm_sec = m8t.datetime.seconds;
	mktime(&temp);
	return temp;
}

time_t gettime_t(Ublox m8t)
{
	tm temp;
	if (!m8t.datetime.valid) return -1;
	temp.tm_year = m8t.datetime.year - 1900 + 2000;
	temp.tm_mon = m8t.datetime.month - 1;
	temp.tm_mday = m8t.datetime.day;
	temp.tm_hour = m8t.datetime.hours;
	temp.tm_min = m8t.datetime.minutes;
	temp.tm_sec = m8t.datetime.seconds;
	return mktime(&temp);
}

void dbgDateTime(const tm* dateTime, u16 millis)
{
	int ret, size;
	size = strftime(dateTimeStr, sizeof(dateTimeStr), "Date: %F\r\nTime: %X", dateTime);


	dbgMsgDateTime.address = (u32)dateTimeStr;
	dbgMsgDateTime.size = strlen(dateTimeStr);
	ret = xQueueSend(tskDbgUartMsgQ, (void *)&dbgMsgDateTime, (TickType_t)10);
//	configASSERT(ret);


	sprintf(millisStr, ":%d\r\n", millis);
	dbgMsgMillis.address = (u32)millisStr;
	dbgMsgMillis.size = strlen(millisStr);
	ret = xQueueSend(tskDbgUartMsgQ, (void *)&dbgMsgMillis, (TickType_t)10);
//	configASSERT(ret);
}

void dbgDateTime(const tm* dateTime)
{
	int ret, size;
	size = strftime(dateTimeStr, sizeof(dateTimeStr), "Date: %F\r\nTime: %X\r\n", dateTime);


	dbgMsgDateTime.address = (u32)dateTimeStr;
	dbgMsgDateTime.size = strlen(dateTimeStr);
	ret = xQueueSend(tskDbgUartMsgQ, (void *)&dbgMsgDateTime, (TickType_t)10);
//	configASSERT(ret);

}

void dbgPosition(float altitude, float latitude, float longtitude)
{
	int ret;
	sprintf(postionStr, "Altitude: %f\r\nLatitude: %f\r\nLongtitude: %f\r\n", altitude, latitude, longtitude);
	dbgMsgPosition.address = (u32)postionStr;
	dbgMsgPosition.size = strlen(postionStr);
	ret = xQueueSend(tskDbgUartMsgQ, (void *)&dbgMsgPosition, (TickType_t)10);
//	configASSERT(ret);
}

void dbgGpsInfo(char* info)
{
	int ret;

	dbgMsgGpsInfo.address = (u32)info;
	dbgMsgGpsInfo.size = strlen(info);
	ret = xQueueSend(tskDbgUartMsgQ, (void *)&dbgMsgGpsInfo, (TickType_t)10);
//	configASSERT(ret);

}

void waitGpsl()
{
	char c;
    while((c = gpsGetChar()) != '\n');
}

void getGpsl(char *line)
{
    int c;
    if(line == NULL)
        return;

    while(1)
    {
        c = gpsGetChar();
        if(c == EOF)
            break;

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return;
}



//void uartGpsIsr(void *CallBackRef, u32 Event, u32 EventData)
//{
//	int ret;
//    BaseType_t wake = 0;
//    u32 IsrStatus;
//
//    if (Event == XUARTPS_EVENT_RECV_DATA)
//    {
//    	if(checkUart()) waitGpsl();
//    	else {
//			getGpsl(gpsInfo);
//			ret = xSemaphoreGiveFromISR(SemRecvGps, &wake);
//			portYIELD_FROM_ISR(wake);
//    	}
//    }
//
////    if(Event == XUARTPS_EVENT_RECV_ERROR)
////    {
////    	uartGps->XilUartpsResetHw();
////    }
//
//}

static void tskGps(void *arg)
{
	int ret;

	waitGpsl();
//	uartGps->XilUartpsRegisterInterrupt(intc, uartGpsIsr);
//	uartGps->XilUartpsSetIntMask(XUARTPS_IXR_RXOVR);
//	uartGps->XilUartpsSetFifoThreshold(2);
	Ublox m8t;
	tm temp;
	isDbgGpsInfo = 0;
	int m8tEncodRet = 0;
	while(1)
	{
		getGpsl(gpsInfo);
		//if(isDbgGpsInfo) dbgGpsInfo(gpsInfo);

        for(int i = 0; i < strlen(gpsInfo); i++){
        	m8tEncodRet = m8t.encode(gpsInfo[i]);
        }

        if(m8tEncodRet == RECV_MSG_GGA)		// Receive GGA
        {
        	ctrlReg.curAltitude = m8t.altitude;
        	ctrlReg.curLatitude = m8t.latitude;
        	ctrlReg.curLongitude = m8t.longitude;
        }
		
        else if(m8tEncodRet == RECV_MSG_RMC)		// Receive RMC
        {
            if(m8t.datetime.valid){
				//temp=getTm(m8t);
				
				temp.tm_year = m8t.datetime.year - 1900 + 2000;
				temp.tm_mon = m8t.datetime.month - 1;
				temp.tm_mday = m8t.datetime.day;
				temp.tm_hour = m8t.datetime.hours+8;
				temp.tm_min = m8t.datetime.minutes;
				temp.tm_sec = m8t.datetime.seconds;
				mktime(&temp);
				
            	ctrlReg.curTime = temp;
            }
        }
		

        vTaskDelay(10);
//        xSemaphoreTake(SemRecvGps, portMAX_DELAY);
	}
}

void tskGpsInit()
{
	int ret;
	SemRecvGps = xSemaphoreCreateBinary();
    configASSERT(SemRecvGps);

    uartGps = new XillUartps(XPAR_PS7_UART_0_DEVICE_ID, 9600, intc, XPAR_XUARTPS_0_INTR, NULL);

    ret = xTaskCreate(tskGps, "tskGps", tskGpsStkSize, NULL, tskGpsPrio, &tskGpsHandle);
    configASSERT(ret == pdPASS);
}
