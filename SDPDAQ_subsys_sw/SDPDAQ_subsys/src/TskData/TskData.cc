#include "TskData.h"
#include <xparameters.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "xaxidma.h"
#include <ff.h>
#include "../TskFFT/TskFFT.h"
#include "../TskFFT/FFT.h"
#include "../TskFFT/Complex.h"
#include "../TskSD/SD.h"
#include "../TskSD/TskSD.h"
#include "../TskTransUart/TskTransUart.h"
#include "string.h"
#include <algorithm>
#include <math.h>

using namespace std;
using namespace LoyMath;

u8 databuf[DATABUF_NUM][TOTAL_DATA_BYTE] __attribute__((aligned(128)));
u8 fftbuf[FFTBUF_NUM][TOTAL_FFT_BYTE] __attribute__((aligned(128)));
extern XilIntc* 		intc;
extern XilGpio*			gpioStartADC;
extern XilGpio*			gpioResetFifo;
extern XilGpio*			gpioAdcDmaLastM;
extern XilAxiDma*		adcDataDma;
extern MonoWavHeader 	wavHeader;
const int tskDataPrio = 9;
const int tskDataStkSize = 16384;
static TaskHandle_t tskDataHandle;

const int tskCheckTimePrio = 9;
const int tskCheckTimeStkSize = 8192;
static TaskHandle_t tskCheckTimeHandle;

QueueHandle_t tskDataMsgQ;
SemaphoreHandle_t semADCDma;
SemaphoreHandle_t semSecCnt;
SemaphoreHandle_t semDataBufWr;
SemaphoreHandle_t semDataBufRd;
volatile unsigned int databuf_wr_p=0;
volatile unsigned int fftbuf_wr_p=0;

extern TaskHandle_t tskFFTHandle;
extern ctrlReg_t ctrlReg;
extern QueueHandle_t 	tskDbgUartMsgQ;
extern secCnt* theSecCnt;
extern unsigned int SD_TOTAL_SIZE;
extern unsigned int SD_FREE_SIZE;
extern QueueHandle_t tskWriteMsgQ;

volatile bool doingfft;
unsigned int readSecCnt = 0;
unsigned int dataReadSecCnt = 0;

void secCntCb(void *arg)
{
    int ret;
    BaseType_t wake = 0;
    secCnt *cnt = (secCnt *)arg;
    ret = xSemaphoreGiveFromISR(semSecCnt, &wake);

    portYIELD_FROM_ISR(wake);
}

void adcDmaS2MMIsr(void* arg)
{
	int msk;
    BaseType_t wake = 0;
    XilAxiDma *dma = (XilAxiDma *)arg;
    msk = XAxiDma_IntrGetIrq(dma->returnXAxiDma(), XAXIDMA_DEVICE_TO_DMA);
	if(msk & XAXIDMA_IRQ_IOC_MASK)
	{
	    int ret = xSemaphoreGiveFromISR(semADCDma, &wake);
	    
	    portYIELD_FROM_ISR(wake);
	}
	XAxiDma_IntrAckIrq(dma->returnXAxiDma(), XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
}


/*
 *  Read the ADC data through the ADC DMA to the dst memory
 */
void readADCData(UINTPTR dstPtr, u32 len)
{
	int status;
	do
	{
		status = adcDataDma->DmaSimpleTransfer((UINTPTR)dstPtr, len, XAXIDMA_DEVICE_TO_DMA);
	}while(status != XST_SUCCESS);
	xSemaphoreTake(semADCDma, portMAX_DELAY);
}


void test_acquisition_data(){
	char msg[64];
	tm cur_time=ctrlReg.curTime;
	tm start_time=cur_time;
	tm end_time=cur_time;
	start_time.tm_sec+=5;
	end_time.tm_sec+=125;
	mktime(&start_time);
	mktime(&end_time);
	sprintf(msg,"%4d %d %d %d %d %d",start_time.tm_year+1900,start_time.tm_mon+1,start_time.tm_mday,start_time.tm_hour,start_time.tm_min,start_time.tm_sec);
	printf("config data acquisition start time %s",msg);
	cmdConfigStartTime(msg);
	sprintf(msg,"%4d %d %d %d %d %d",end_time.tm_year+1900,end_time.tm_mon+1,end_time.tm_mday,end_time.tm_hour,end_time.tm_min,end_time.tm_sec);
	printf("config data acquisition end time %s",msg);
	cmdConfigEndTime(msg);
	sprintf(msg,"0");
	cmdSetStart(msg);
	printf("config data acquisition finish\r\n");
}

void test_acquisition_FFT(){
	char msg[64];
	tm cur_time=ctrlReg.curTime;
	tm start_time=cur_time;
	tm end_time=cur_time;
	start_time.tm_sec+=5;
	end_time.tm_sec+=125;
	mktime(&start_time);
	mktime(&end_time);
	sprintf(msg,"%4d %d %d %d %d %d",start_time.tm_year+1900,start_time.tm_mon+1,start_time.tm_mday,start_time.tm_hour,start_time.tm_min,start_time.tm_sec);
	printf("config fft acquisition start time %s",msg);
	cmdConfigStartTime(msg);
	sprintf(msg,"%4d %d %d %d %d %d",end_time.tm_year+1900,end_time.tm_mon+1,end_time.tm_mday,end_time.tm_hour,end_time.tm_min,end_time.tm_sec);
	printf("config fft acquisition end time %s",msg);
	cmdConfigEndTime(msg);
	sprintf(msg,"1");
	cmdSetStart(msg);
	printf("config fft acquisition finish\r\n");
}

static void tskData(void *arg)
{
	char dirName[50];
	char fileName[50];
	unsigned int bw;
	int ret,sts;
	int dataSize = 0;
	int chunkSize = 0;
	unsigned int next_fftbuf_wr_p;
	tskDataMsg_t msg;
	u8 *p;
	while(1)
	{
		xQueueReceive(tskDataMsgQ, &msg, portMAX_DELAY);
		switch(msg.id){
			case doData:
				if(DBG_INFO)
					printf("Read one sec, data\r\n");
				for(int i = 0; i < (ADC_SAMPLE_RATE / NUM_PER_DMA_DATA_OP); i++){
					p=databuf[databuf_wr_p];
					readADCData((UINTPTR)p, TOTAL_DATA_BYTE);
					xSemaphoreGive(semDataBufRd);
					databuf_wr_p=databuf_wr_p==DATABUF_NUM-1?0:databuf_wr_p+1;
				}
				break;
			case dofft:
				if(DBG_INFO)
					printf("Read one sec, fft\r\n");
				gpioAdcDmaLastM->XilGpioDiscreteWrite(1,16384);
				gpioResetFifo->XilGpioDiscreteWrite(1,0);
				gpioResetFifo->XilGpioDiscreteWrite(1,1);
				gpioStartADC->XilGpioDiscreteWrite(1,1);
				fftbuf_wr_p=0;
				p=fftbuf[fftbuf_wr_p];
				readADCData((UINTPTR)p, TOTAL_FFT_BYTE);
				vTaskResume(tskFFTHandle);
				break;
				while(doingfft){
					next_fftbuf_wr_p=fftbuf_wr_p==FFTBUF_NUM-1?0:fftbuf_wr_p+1;
					p=fftbuf[next_fftbuf_wr_p];
					readADCData((UINTPTR)p, TOTAL_FFT_BYTE);
					fftbuf_wr_p=next_fftbuf_wr_p;
				}
				gpioStartADC->XilGpioDiscreteWrite(1,0);
				break;
		}
	}
}

static void tskCheckTime(void *arg)
{
	int ret;
    ctrlReg.dataReading = 0;
	float df;
	tskDataMsg_t tskDataMsg;
	tskWriteMsg_t tskWriteMsg;
	u8 earlyStopPhase = 0;
	unsigned int writeSecCnt;
	theSecCnt = new secCnt(intc, 64, 99000000, XPAR_ADC_HIER_SECCNT_S00_AXI_BASEADDR, secCntCb, &theSecCnt);
    while(1)
    {
    	xSemaphoreTake(semSecCnt, portMAX_DELAY);
    	if(ctrlReg.If_Start){
    		if(ctrlReg.dataReading){
    			df = difftime(mktime(&ctrlReg.endTime), mktime(&ctrlReg.curTime));
    			if(DBG_INFO){
    				//printf("diff:%f\n\r",df);
				}
				if(df == 1){
					ctrlReg.dataReading = 0;
					if(!doingfft){
						gpioStartADC->XilGpioDiscreteWrite(1,0);
					}
					doingfft=false;
					printf("Read command complete\n\r");
				}
				else if(df == 2){
					if(ctrlReg.If_FFT){
						//tskDataMsg.id=dofft;
						//ret = xQueueSend(tskDataMsgQ, (void *)&tskDataMsg, (TickType_t)10);
						//configASSERT(ret==pdPASS);
					}else{
						tskDataMsg.id=doData;
						tskWriteMsg.id = WriteLastSec;
						ret = xQueueSend(tskDataMsgQ, (void *)&tskDataMsg, (TickType_t)10);
						configASSERT(ret==pdPASS);
						ret = xQueueSend(tskWriteMsgQ, (void *)&tskWriteMsg, (TickType_t)10);
						configASSERT(ret==pdPASS);
						writeSecCnt++;
					}
				}
				else if( writeSecCnt == (ctrlReg.secondsIncrease-1) ){
					if(ctrlReg.If_FFT){
						//tskDataMsg.id=dofft;
						//ret = xQueueSend(tskDataMsgQ, (void *)&tskDataMsg, (TickType_t)10);
						//configASSERT(ret==pdPASS);
					}else{
						tskDataMsg.id=doData;
						tskWriteMsg.id = WriteChangeFile;
						ret = xQueueSend(tskDataMsgQ, (void *)&tskDataMsg, (TickType_t)10);
						configASSERT(ret==pdPASS);
						ret = xQueueSend(tskWriteMsgQ, (void *)&tskWriteMsg, (TickType_t)10);
						configASSERT(ret==pdPASS);
						writeSecCnt=0;
					}
	    		}
				else{
					if(ctrlReg.If_FFT){
						//tskDataMsg.id=dofft;
						//ret = xQueueSend(tskDataMsgQ, (void *)&tskDataMsg, (TickType_t)10);
						//configASSERT(ret==pdPASS);
					}else{
						tskDataMsg.id=doData;
						tskWriteMsg.id = WriteOneSec;
						ret = xQueueSend(tskDataMsgQ, (void *)&tskDataMsg, (TickType_t)10);
						configASSERT(ret==pdPASS);
						ret = xQueueSend(tskWriteMsgQ, (void *)&tskWriteMsg, (TickType_t)10);
						configASSERT(ret==pdPASS);
						writeSecCnt++;
					}
				}
    		}
    		else{
    			df = difftime(mktime(&ctrlReg.startTime), mktime(&ctrlReg.curTime));
    			if(df == 1){
					writeSecCnt=0;
    	    		ctrlReg.dataReading = 1;
					printf("Command start\n\r");
					if(ctrlReg.If_FFT){
						tskDataMsg.id=dofft;
						doingfft=true;
						ret = xQueueSend(tskDataMsgQ, (void *)&tskDataMsg, (TickType_t)10);
						configASSERT(ret==pdPASS);
					}else{
						tskDataMsg.id=doData;
						tskWriteMsg.id = WriteFirstSec;
						gpioAdcDmaLastM->XilGpioDiscreteWrite(1,NUM_PER_DMA_DATA_OP);
						gpioResetFifo->XilGpioDiscreteWrite(1,0);
						gpioResetFifo->XilGpioDiscreteWrite(1,1);
						gpioStartADC->XilGpioDiscreteWrite(1,1);
						ret = xQueueSend(tskDataMsgQ, (void *)&tskDataMsg, (TickType_t)10);
						configASSERT(ret==pdPASS);
						ret = xQueueSend(tskWriteMsgQ, (void *)&tskWriteMsg, (TickType_t)10);
						configASSERT(ret==pdPASS);
						writeSecCnt++;
					}
    			}
    		}
    	}
    	else{
    		if(!ctrlReg.adcTesting && !ctrlReg.fftTesting){
				if(ctrlReg.dataReading){											// Stoped at the middle of the acquisition
	    			if(DBG_INFO)
	    				printf("stop at middle of acquisition, readSecCnt:%d\r\n", writeSecCnt);
	    			if(earlyStopPhase == 0){
						tskDataMsg.id=ctrlReg.If_FFT?dofft:doData;
						tskWriteMsg.id = WriteLastSec;
						ret = xQueueSend(tskDataMsgQ, (void *)&tskDataMsg, (TickType_t)10);
						configASSERT(ret==pdPASS);
						ret = xQueueSend(tskWriteMsgQ, (void *)&tskWriteMsg, portMAX_DELAY);
						earlyStopPhase = 1;
						writeSecCnt++;
	    			}
	    			else {
						ctrlReg.dataReading = 0;
						earlyStopPhase = 0;
	    			}
				}
				else{
					writeSecCnt = 0;
				}
    		}
    		else {
    			writeSecCnt = 0;
    		}
    	}
    }
}

void tskDataInit()
{
	int ret;
	semADCDma = xSemaphoreCreateBinary();
    configASSERT(semADCDma);

    semSecCnt = xSemaphoreCreateBinary();
    configASSERT(semSecCnt);
	
    semDataBufWr=xSemaphoreCreateCounting(DATABUF_NUM,DATABUF_NUM);
    configASSERT(semDataBufWr);

    semDataBufRd=xSemaphoreCreateCounting(DATABUF_NUM,0);
    configASSERT(semDataBufRd);

	tskDataMsgQ=xQueueCreate(120,sizeof(tskDataMsg_t));
	configASSERT(tskDataMsgQ);

	XAxiDma_Reset(adcDataDma->returnXAxiDma());
	while(!XAxiDma_ResetIsDone(adcDataDma->returnXAxiDma()));

	intc->InitChannelInt(XPAR_FABRIC_AXIDMA_0_VEC_ID, (XInterruptHandler)&adcDmaS2MMIsr, (void *)adcDataDma);
	XAxiDma_IntrEnable(adcDataDma->returnXAxiDma(), XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);

    ret = xTaskCreate(tskData, "TskData", tskDataStkSize, NULL, tskDataPrio, &tskDataHandle);
    configASSERT(ret==pdPASS);

	ret = xTaskCreate(tskCheckTime, "TskCheckTime", tskCheckTimeStkSize, NULL, tskCheckTimePrio, &tskCheckTimeHandle);
    configASSERT(ret==pdPASS);
}
