#include <xsdps.h>
#include <xil_assert.h>
#include <ff.h>
#include "SD.h"

int SDinit()
{
    int sts=0;
    XSdPs_Config *sdconf;
    XSdPs sd;
    FATFS fat;
    sdconf = XSdPs_LookupConfig(XPAR_XSDPS_0_DEVICE_ID);
    sts = XSdPs_CfgInitialize(&sd, sdconf, XPAR_XSDPS_0_BASEADDR);
    sts |= XSdPs_CardInitialize(&sd);
    sts |= f_mount(&fat, "", 1);

    return sts;
}

extern ctrlReg_t ctrlReg;
int SDTest(){
    tm curTime=ctrlReg.curTime;
	FIL wf;
	unsigned int bw;
    FILINFO file_info;
	int sts=0;
    char msg[64]="test SD successful";
	sts=f_open(&wf, "testSD.txt", FA_OPEN_APPEND | FA_WRITE);
    configASSERT(sts==FR_OK);
    sts=f_write(&wf, msg, strlen(msg), &bw);
    configASSERT(bw==strlen(msg));
    sts=f_close(&wf);
    configASSERT(sts==FR_OK);
    f_stat("testSD.txt",&file_info);
    printf("%hu,%hu\n\r",file_info.fdate,file_info.ftime);
    printf("%4d %d %d %d %d %d",curTime.tm_year,curTime.tm_mon,curTime.tm_mday,curTime.tm_hour,curTime.tm_min,curTime.tm_sec);
    //file_info.fdate=(((curTime.tm_year+1900-1980)<<9)|((curTime.tm_mon+1)<<5)|(curTime.tm_mday))&0xffff;
    file_info.ftime=(((curTime.tm_hour+16)%24)<<11)|(curTime.tm_min<<5)|(curTime.tm_sec/2);
    f_utime("testSD.txt",&file_info);
    f_stat("testSD.txt",&file_info);
    printf("%hu,%hu\n\r",file_info.fdate,file_info.ftime);
}

