#include <FreeRTOS.h>
#include <semphr.h>
#include <ff.h>
#include <xsdps.h>
#include "../common.h"
#include "../TskData/TskData.h"
#include "../TskHif/TskHif.h"

const int tskSDPrio = 7;
const int tskSDStkSize = 16384;
static TaskHandle_t tskSDHandle;

SemaphoreHandle_t semSDReady;
unsigned int databuf_rd_p=0;

MonoWavHeader wavHeader;

FATFS fat;
FILINFO file;
FIL wf;
DIR dir;
XSdPs_Config *sdconf;
XSdPs sd;

void initSD(){
    int sts=0;
    sdconf=XSdPs_LookupConfig(XPAR_XSDPS_0_DEVICE_ID);
    XSdPs_CfgInitialize(&sd, sdconf, XPAR_XSDPS_0_BASEADDR);
    tfCardSta = 0x01;
    //sts=XSdPs_CardInitialize(&sd);
    sts=f_mount(&fat,(const TCHAR *)(""),1);
    if(sts!=FR_OK){
        DCM_RED_LED_ON;
        do
        {
            if(DEBUG)
                printf("try initialize SD card\n\r");
            //sts=XSdPs_CardInitialize(&sd);
            sts=f_mount(&fat,(const TCHAR *)(""),1);
            vTaskDelay(500);
            tfCardSta = 0x01;
        }while(sts!=FR_OK);
    }
    DCM_RED_LED_OFF;
	tfCardSta = 0x02; // tf card ok
}

void setFsTimeStamp(char* fileNamePtr){
	FILINFO file_info;
    DateTime32 dcm;
    dcm.Value=DateTime.Value;
	f_stat(fileNamePtr,&file_info);
    file_info.fdate=((dcm.YearFrom2000+1940)<<9)|(dcm.Month<<5)|(dcm.Day);
    file_info.ftime=((dcm.Hour%24)<<11)|((dcm.Minute)<<5)|(dcm.Second>>1);
    f_utime(fileNamePtr,&file_info);
}

static void tskSD(void* pvParameters){
	u8* p;
	DateTime32 dcm;
	int sts,ret;
	FIL wf;
    DIR dir;
    unsigned int bw;
    char dirName[48],fileName[48];
	int dataSize=0;
	int chunkSize=0;
    unsigned int cnt=0,round=0;

    DMC_ALL_LED_OFF;
    vTaskDelay(750);
    DCM_RED_LED_ON;
    vTaskDelay(750);
    DCM_GREEN_LED_ON;
    vTaskDelay(750);
    DCM_YELLOW_LED_ON;
    vTaskDelay(750);
    DMC_ALL_LED_OFF;

    initSD();
    DCM_YELLOW_LED_ON;
	xSemaphoreGive(semSDReady);
	while(1){
        dcm.Value = DateTime.Value;
        sts=f_chdir((const TCHAR *)("/"));
        sprintf(dirName, "%04d%02d%02d", (int)(dcm.YearFrom2000 + 2000), (int)dcm.Month, (int)dcm.Day);
        sts = f_opendir(&dir, (const TCHAR *)dirName);
        if(sts != FR_OK){
            sts = f_mkdir((const TCHAR *)dirName);
            setFsTimeStamp(dirName);
        }
        sts = f_chdir((const TCHAR *)dirName);
		sprintf(fileName, "%02d%02d%02d.wav",
				(int)dcm.Hour, (int)dcm.Minute, (int)dcm.Second);
		sts=f_open(&wf,fileName,FA_CREATE_ALWAYS | FA_WRITE);
		sts=f_write(&wf, (void *)&wavHeader, sizeof(MonoWavHeader), &bw);
		round=0;cnt=0;
        for(int m=0;m<SECONDS_PER_FILE;m++){
			if(round%4==0) DCM_GREEN_TOGGLE;
			for(int i=0;i<SAMPLE_RATE/DMA_TRANS_SIZE;i++){
				xSemaphoreTake(semDataBufRd,portMAX_DELAY);
				p=databuf[databuf_rd_p];
				Xil_DCacheInvalidateRange((UINTPTR)p, DMA_TRANS_SIZE*3);
				f_write(&wf,(void*)p,3*DMA_TRANS_SIZE,&bw);
				xSemaphoreGive(semDataBufWr);
				databuf_rd_p=databuf_rd_p==DATABUF_NUM-1?0:databuf_rd_p+1;
				cnt++;
			}
			round++;
            if(round%10==0 || round==SECONDS_PER_FILE){
                dataSize=DMA_TRANS_SIZE*3*cnt;
                chunkSize=dataSize + 36;
                sts = f_lseek(&wf, (int)&(((MonoWavHeader *)0)->ChunkSize));
                sts |= f_write(&wf, &chunkSize, 4, &bw);
                sts |= f_lseek(&wf, (int)&(((MonoWavHeader *)0)->Subchunk2Size));
                sts |= f_write(&wf, &dataSize, 4, &bw);
                sts |= f_lseek(&wf, f_size(&wf));
                sts |= f_sync(&wf);
            }
		}
        sts = f_close(&wf);
        setFsTimeStamp(fileName);
	}
}

void tskSDInit()
{
	int ret;

	semSDReady=xSemaphoreCreateBinary();
    configASSERT(semSDReady);

    ret=xTaskCreate(tskSD,"TskSD",tskSDStkSize,NULL,tskSDPrio,&tskSDHandle);
    configASSERT(ret==pdPASS);
}
