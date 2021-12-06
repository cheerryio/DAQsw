#include <FreeRTOS.h>
#include <semphr.h>
#include <ff.h>
#include <xsdps.h>
#include "../common.h"
#include "../TskData/TskData.h"
#include "../TskSD/SD.h"

const int tskSDPrio = 8;
const int tskSDStkSize = 16384;
static TaskHandle_t tskSDHandle;

SemaphoreHandle_t semSDReady;
unsigned int databuf_rd_p=0;
MonoWavHeader wavHeader;

QueueHandle_t tskWriteMsgQ;
extern QueueHandle_t tskDataMsgQ;
extern XilIntc* intc;
extern ctrlReg_t ctrlReg;
unsigned int file_seq = 1;
char storeInfo[200];

void storePosStartInformation(char* fileName){
	u32 size;
	FIL wf;

    int sts=0;
    unsigned int bw;
    sts = f_open(&wf, fileName, FA_CREATE_ALWAYS | FA_WRITE);
    
    sprintf(storeInfo, "Acquisition Start\r\n");
    sprintf(&storeInfo[19], "Altitude: %f\r\nLatitude: %f\r\nLongtitude: %f\r\n", ctrlReg.curAltitude, ctrlReg.curLatitude, ctrlReg.curLongitude);
    sts = f_write(&wf, storeInfo, strlen(storeInfo), &bw);
    configASSERT(sts==FR_OK);
    
	sts=f_lseek(&wf, f_size(&wf));
	configASSERT(sts==FR_OK);
	sts=f_sync(&wf);
	configASSERT(sts==FR_OK);
    sts = f_close(&wf);
    configASSERT(sts==FR_OK);
}

void createRootDirWithPos(char* dirNamePtr, char* fileNamePtr, FIL *wf){
	int sts;
	DIR dir;
	sprintf(dirNamePtr, "/");
	sts=f_opendir(&dir,dirNamePtr);
	configASSERT(sts==FR_OK);
	tm cur_time=ctrlReg.curTime;
	sprintf(fileNamePtr, "location.txt");
	storePosStartInformation(fileNamePtr);
	file_seq=1;
	sprintf(fileNamePtr, "Data-%4d-%d-%d-%d-%d-%d.wav", cur_time.tm_year + 1900, cur_time.tm_mon + 1, cur_time.tm_mday,
														cur_time.tm_hour, cur_time.tm_min, file_seq);
    unsigned int bw;
    sts = f_open(wf, fileNamePtr, FA_CREATE_ALWAYS | FA_WRITE);
	configASSERT(sts==FR_OK);
	sts = f_write(wf,(void *)&wavHeader,sizeof(MonoWavHeader),&bw);
	configASSERT(sts==FR_OK && bw == sizeof(MonoWavHeader));
}

void changeFile(char* fileNamePtr, FIL *wf){
	int sts;
	file_seq++;
	sprintf(fileNamePtr, "Data-%4d-%d-%d-%d-%d-%d.wav", ctrlReg.curTime.tm_year + 1900, ctrlReg.curTime.tm_mon + 1, ctrlReg.curTime.tm_mday,
														ctrlReg.curTime.tm_hour, ctrlReg.curTime.tm_min,file_seq);
    unsigned int bw;
    sts = f_open(wf, fileNamePtr, FA_CREATE_ALWAYS | FA_WRITE);
	configASSERT(sts==FR_OK);
    sts = f_write(wf, (void *)&wavHeader, sizeof(MonoWavHeader), &bw);
	configASSERT(sts==FR_OK);
}

void writeOneSecSimple(FIL *wf){
	u8 *p;
	unsigned int bw;
	for(int i=0;i<(ADC_SAMPLE_RATE / NUM_PER_DMA_DATA_OP);i++){
		xSemaphoreTake(semDataBufRd,portMAX_DELAY);
		p=databuf[databuf_rd_p];
		Xil_DCacheInvalidateRange((UINTPTR)p, TOTAL_DATA_BYTE);
		f_write(wf,(void*)p,TOTAL_DATA_BYTE,&bw);
		xSemaphoreGive(semDataBufWr);
		databuf_rd_p=databuf_rd_p==DATABUF_NUM-1?0:databuf_rd_p+1;
	}
}

void closeWavFile(FIL *wf,unsigned int dataSize,unsigned int chunkSize){
	int ret;
	unsigned int bw;
	ret = f_lseek(wf, (int)&(((MonoWavHeader *)0)->ChunkSize));
	configASSERT(ret==FR_OK);
	ret = f_write(wf, &chunkSize, 4, &bw);
	configASSERT(ret==FR_OK);
	ret = f_lseek(wf, (int)&(((MonoWavHeader *)0)->Subchunk2Size));
	configASSERT(ret==FR_OK);
	ret = f_write(wf, &dataSize, 4, &bw);
	configASSERT(ret==FR_OK);
	ret = f_lseek(wf, f_size(wf));
	configASSERT(ret==FR_OK);
	ret = f_sync(wf);
	configASSERT(ret==FR_OK);
	ret = f_close(wf);
	configASSERT(ret==FR_OK);
}

static void tskSD(void* pvParameters){
	u8* p;
	int sts,ret;
	FIL wf;
    DIR dir;
    unsigned int bw;
    char dirName[48],fileName[48];
	int dataSize=0;
	int chunkSize=0;
	tskWriteMsg_t msg;
    unsigned int cnt=0,round=0;

	while(1){
		xQueueReceive(tskWriteMsgQ, &msg, portMAX_DELAY);
		switch(msg.id){
			case WriteOneSec:
				if(DBG_INFO) printf("Write one sec\n\r");
				writeOneSecSimple(&wf);
				cnt++;
				break;
			case WriteFirstSec:
				if(DBG_INFO) printf("Write first sec\n\r");
				createRootDirWithPos(dirName, fileName, &wf);
				cnt=0;
				writeOneSecSimple(&wf);
				cnt++;
				break;
			case WriteLastSec:
				if(DBG_INFO) printf("Write last sec\n\r");
				writeOneSecSimple(&wf);
				cnt++;
				dataSize=ADC_SAMPLE_RATE*DATA_BYTE_NUM * cnt;
				chunkSize=dataSize + 36;
				closeWavFile(&wf,dataSize,chunkSize);
				printf("Write Data Ends\n\r");
				break;
			case WriteChangeFile:
				if(DBG_INFO) printf("Write change file\n\r");
				writeOneSecSimple(&wf);
				cnt++;
				dataSize=ADC_SAMPLE_RATE*DATA_BYTE_NUM * cnt;
				chunkSize=dataSize + 36;
				closeWavFile(&wf,dataSize,chunkSize);
				changeFile(fileName, &wf);
				cnt=0;
				break;
			default:
				break;
		}
	}
}

void tskSDInit()
{
	int ret,sts;

	sts=SDinit();
	if(sts!=FR_OK){
		printf("SDinit fail\n\r");
	}

    tskWriteMsgQ = xQueueCreate(128, sizeof(tskWriteMsg_t));
	configASSERT(tskWriteMsgQ);

    ret=xTaskCreate(tskSD,"TskSD",tskSDStkSize,NULL,tskSDPrio,&tskSDHandle);
    configASSERT(ret==pdPASS);
}
