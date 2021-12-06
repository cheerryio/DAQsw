#include <xsdps.h>
#include <xil_assert.h>
#include <ff.h>
#include "SD.h"

FATFS fat;
FILINFO file;
FIL wf;
DIR dir;
XSdPs_Config *sdconf;
XSdPs sd;

MonoWavHeader wavHeader;

int SDinit()
{
    int sts=0;
    sdconf = XSdPs_LookupConfig(XPAR_XSDPS_0_DEVICE_ID);

    sts = XSdPs_CfgInitialize(&sd, sdconf, XPAR_XSDPS_0_BASEADDR);
    Xil_AssertNonvoid(sts == XST_SUCCESS);

    sts = XSdPs_CardInitialize(&sd);
    Xil_AssertNonvoid(sts == XST_SUCCESS);

    sts = f_mount(&fat, "", 1);
    Xil_AssertNonvoid(sts == FR_OK);
    return FR_OK;
}


int SDRead(int *data, unsigned int len, char *fileName)
{
    int sts=0;
    sts = f_open(&wf, fileName, FA_READ );
    Xil_AssertNonvoid(sts == FR_OK);
    unsigned int bw;
    sts = f_read(&wf, data, len*4, &bw);
    Xil_AssertNonvoid(bw == len*4);
    Xil_AssertNonvoid(sts == FR_OK);
    sts = f_close(&wf);
    Xil_AssertNonvoid(sts == FR_OK);
    return FR_OK;
}

int SDWriteInt(int *data, unsigned int len, char *fileName)
{
    int sts=0;
    unsigned int bw;
    sts = f_open(&wf, fileName, FA_OPEN_APPEND | FA_WRITE);
    Xil_AssertNonvoid(sts == FR_OK);
    sts = f_write(&wf, data, len*4, &bw);
    Xil_AssertNonvoid(bw == len*4);
    Xil_AssertNonvoid(sts == FR_OK);
    sts = f_close(&wf);
    Xil_AssertNonvoid(sts == FR_OK);
    return FR_OK;
}

int SDWriteFloat(float *data, unsigned int len, char *fileName)
{
    int sts=0;
    unsigned int bw;
    sts = f_open(&wf, fileName, FA_CREATE_ALWAYS | FA_WRITE);
    Xil_AssertNonvoid(sts == FR_OK);
    sts = f_write(&wf, data, len*4, &bw);
    Xil_AssertNonvoid(bw == len*4);
    Xil_AssertNonvoid(sts == FR_OK);
    sts = f_close(&wf);
    Xil_AssertNonvoid(sts == FR_OK);
    return FR_OK;
}


int SDWriteDouble(double *data, unsigned int len, char *fileName)
{
    int sts=0;
    unsigned int bw;
    sts = f_open(&wf, fileName, FA_OPEN_APPEND | FA_WRITE);
    Xil_AssertNonvoid(sts == FR_OK);
    sts = f_write(&wf, data, len*8, &bw);
    Xil_AssertNonvoid(bw == len*8);
    Xil_AssertNonvoid(sts == FR_OK);
    sts = f_close(&wf);
    Xil_AssertNonvoid(sts == FR_OK);
    return FR_OK;
}


int SDWriteString(char *data, unsigned int len, char *fileName)
{
    int sts=0;
    unsigned int bw;
    sts = f_open(&wf, fileName, FA_OPEN_APPEND | FA_WRITE);
    Xil_AssertNonvoid(sts == FR_OK);
    sts = f_write(&wf, data, len, &bw);
    Xil_AssertNonvoid(bw == len);
    Xil_AssertNonvoid(sts == FR_OK);
    sts = f_close(&wf);
    Xil_AssertNonvoid(sts == FR_OK);
    return FR_OK;
}

int SDWriteWavHeader(char *fileName)
{
    int sts=0;
    unsigned int bw;
    sts = f_open(&wf, fileName, FA_CREATE_ALWAYS | FA_WRITE);
    Xil_AssertNonvoid(sts == FR_OK);
    sts = f_write(&wf, (void *)&wavHeader, sizeof(MonoWavHeader), &bw);
	configASSERT(sts == FR_OK && bw == sizeof(MonoWavHeader));
    sts = f_close(&wf);
    Xil_AssertNonvoid(sts == FR_OK);
	return FR_OK;
}

int openDir(char *dirName)
{
	int sts=0;

	sts = f_opendir(&dir, (const TCHAR *)dirName);
	if(sts != FR_OK){
		sts = f_mkdir((const TCHAR *)dirName);
		Xil_AssertNonvoid(sts == FR_OK);
		sts = f_chdir((const TCHAR *)dirName);
		Xil_AssertNonvoid(sts == FR_OK);
	}

	return FR_OK;
}

int closeDir()
{
	int sts=0;
	sts = f_closedir(&dir);
	Xil_AssertNonvoid(sts == FR_OK);

	return FR_OK;
}

int openFile(char *fileName)
{
	int sts=0;
	sts = f_open(&wf, fileName, FA_OPEN_APPEND | FA_WRITE);
	Xil_AssertNonvoid(sts == FR_OK);
	return FR_OK;
}

int createFile(char *fileName)
{
	int sts=0;
	sts = f_open(&wf, fileName, FA_CREATE_ALWAYS | FA_WRITE);
	Xil_AssertNonvoid(sts == FR_OK);
	return FR_OK;
}

int closeFile(char *fileName)
{
	int sts=0;
	sts = f_close(&wf);
	Xil_AssertNonvoid(sts == FR_OK);
	return FR_OK;
}

int nativeWrite(char *data, unsigned int len)
{
	int sts=0;
	unsigned int bw;
	sts = f_write(&wf, data, len, &bw);
	configASSERT(bw == len);
	return FR_OK;
}

int fileSeek(unsigned int pos)
{
	int sts=0;
	f_lseek(&wf, pos);
	configASSERT(sts == FR_OK);
	return FR_OK;
}

int fileSync()
{
	int sts=0;
	f_sync(&wf);
	configASSERT(sts == FR_OK);
	return FR_OK;
}

int seekFileEnd()
{
	int sts=0;
	f_lseek(&wf, f_size(&wf));
	configASSERT(sts == FR_OK);
	return FR_OK;
}

unsigned int SD_TOTAL_SIZE = 0;
unsigned int SD_FREE_SIZE = 0;

int getSDFreeCapacity(void)
{
  FATFS *pfs;
  DWORD fre_clust;
  BYTE res;

  pfs=&fat;

  res = f_getfree("", &fre_clust, &pfs);
  if (res) return res; // 如果f_getfree函数执行错误，返回错误值
  else // 如果成功获得容量
  {
    SD_TOTAL_SIZE = (pfs->n_fatent - 2) * pfs->csize/2; //总容量 单位byte
    SD_FREE_SIZE = fre_clust * pfs->csize/2; // 可用容量 单位byte
  }
  //f_unmount(""); //卸载文件系统
  return 0;
}

int SDTest(){
	FIL wf;
	unsigned int bw;
	int sts=0;
    char msg[64]="LPDAQ subsys test SD successful";
	sts=f_open(&wf, "testSD.txt", FA_OPEN_APPEND | FA_WRITE);
    configASSERT(sts==FR_OK);
    sts=f_write(&wf, msg, strlen(msg), &bw);
    configASSERT(bw==strlen(msg));
    sts=f_close(&wf);
    configASSERT(sts==FR_OK);
}

