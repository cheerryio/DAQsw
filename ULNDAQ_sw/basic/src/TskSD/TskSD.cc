#include <FreeRTOS.h>
#include <semphr.h>
#include <ff.h>
#include <xsdps.h>
#include <event_groups.h>
#include "../common.h"
#include "../TskData/TskData.h"
#include "../TskHif/TskHif.h"

const int tskSDPrio = 8;
const int tskSDStkSize = 4096;
TaskHandle_t tskSDHandle;
tskSDCmd_t cmds[CHANNEL_NUM];
SemaphoreHandle_t semSDReady;
unsigned int databuf_rd_p[CHANNEL_NUM]={0};
EventGroupHandle_t writeDoneEvents;

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
	FIL wf[CHANNEL_NUM];
    DIR dir;
    unsigned int bw;
    char dirName[48],fileName[CHANNEL_NUM][48];
	int dataSize=0;
	int chunkSize=0;
    unsigned int cnt=0,round=0;
    unsigned int buffered_cnt;

    DMC_ALL_LED_OFF;
    vTaskDelay(1000);
    DCM_RED_LED_ON;
    vTaskDelay(1000);
    DCM_GREEN_LED_ON;
    vTaskDelay(1000);
    DCM_YELLOW_LED_ON;
    vTaskDelay(1000);
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
        for(int i=0;i<CHANNEL_NUM;i++){
            sprintf(fileName[i], "%02d%02d%02d-%d.wav",
                    (int)dcm.Hour, (int)dcm.Minute, (int)dcm.Second, i);
            sts=f_open(&wf[i],fileName[i],FA_CREATE_ALWAYS | FA_WRITE);
            sts=f_write(&wf[i], (void *)&wavHeader, sizeof(MonoWavHeader), &bw);
        }
		round=0;cnt=0;
        buffered_cnt=0;
        for(int m=0;m<SECONDS_PER_FILE;m++){
			if(m%4==0) DCM_GREEN_TOGGLE;
			for(int n=0;n<REALIGNED_SAMPLE_RATE/DMA_TRANS_SIZE;n++){
                if(buffered_cnt<SD_BUFFERED_NUM-1){
                    if(DEBUG)
                        printf("buffer write file\n\r");
                    for(int i=0;i<CHANNEL_NUM;i++){
                        xSemaphoreTake(semDataBufRd[i],portMAX_DELAY);
                    }
                    buffered_cnt++;
                }
                else{
                    if(DEBUG)
                        printf("real write file\n\r");
                    for(int i=0;i<CHANNEL_NUM;i++){
                        xSemaphoreTake(semDataBufRd[i],portMAX_DELAY);
                        p=databuf[i][databuf_rd_p[i]];
                        Xil_DCacheInvalidateRange((UINTPTR)p, DMA_TRANS_SIZE*4*SD_BUFFERED_NUM);
                        f_write(&wf[i],(void*)p,DMA_TRANS_SIZE*4*SD_BUFFERED_NUM,&bw);
                        for(int v=0;v<SD_BUFFERED_NUM;v++){
                            xSemaphoreGive(semDataBufWr[i]);
                            databuf_rd_p[i]=databuf_rd_p[i]==DATABUF_NUM-1?0:databuf_rd_p[i]+1;
                        }
                    }
                    cnt+=SD_BUFFERED_NUM;
                    buffered_cnt=0;
                }
			}
			round++;
            if(round%10==0 || round==SECONDS_PER_FILE){
                dataSize=DMA_TRANS_SIZE*4*cnt;
                chunkSize=dataSize + 36;
                for(int i=0;i<CHANNEL_NUM;i++){
                    sts = f_lseek(&wf[i], (int)&(((MonoWavHeader *)0)->ChunkSize));
                    sts |= f_write(&wf[i], &chunkSize, 4, &bw);
                    sts |= f_lseek(&wf[i], (int)&(((MonoWavHeader *)0)->Subchunk2Size));
                    sts |= f_write(&wf[i], &dataSize, 4, &bw);
                    sts |= f_lseek(&wf[i], f_size(&wf[i]));
                    sts |= f_sync(&wf[i]);
                    setFsTimeStamp(fileName[i]);
                }
            }
		}
        for(int i=0;i<CHANNEL_NUM;i++){
            sts = f_close(&wf[i]);
        }
	}

    vTaskDelete(NULL);
}

void tskSDTest(void* pvParameters){
    tskSDCmd_t* cmd=(tskSDCmd_t*)pvParameters;
    unsigned int channelNum=cmd->channnelNum;
    unsigned int writeDoneBits=0;
    for(int i=0;i<CHANNEL_NUM;i++){
        writeDoneBits|=(1<<i);
    }
    while(1){
        vTaskDelay(1);
        printf("%d\n\r",channelNum);
        xEventGroupSetBits(writeDoneEvents,1<<channelNum);
        xEventGroupWaitBits(writeDoneEvents,writeDoneBits,pdFALSE,pdTRUE,portMAX_DELAY);
        xEventGroupClearBits(writeDoneEvents,writeDoneBits);
    }

    vTaskDelete(NULL);
}

void tskSDInit()
{
	int ret;

	semSDReady=xSemaphoreCreateBinary();
    configASSERT(semSDReady);

    writeDoneEvents=xEventGroupCreate();
    configASSERT(writeDoneEvents);

    ret=xTaskCreate(tskSD,"TskSD",tskSDStkSize,NULL,tskSDPrio,&tskSDHandle);
    configASSERT(ret==pdPASS);
}
