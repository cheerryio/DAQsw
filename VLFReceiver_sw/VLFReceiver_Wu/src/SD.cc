/*
 * SD.c
 *
 *  Created on: Mar 5, 2021
 *      Author: WuXJ
 *
 */

#include "ff.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "stdio.h"
#include "SD.h"
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "TskHif/TskHif.h"
#include "tskData.h"
#include "Queue.h"
#include "xtime_l.h"
#include "sleep.h"
const int tskSDPrio = 5;
const int tskSDStkSize = 16384 * 2;

TaskHandle_t tskSDHandle;

#pragma pack(push)
#pragma pack(1)
struct MonoWavHeader
{
	char ChunkID[4];// = {'R','I','F','F'};
	unsigned int ChunkSize;// = 36 + data bytes;
	char Format[4];// = {'W','A','V','E'};
	char Subchunk1ID[4];// = {'f','m','t',' '};
	unsigned int Subchunk1Size;// = 16;
	unsigned short AudioFormat;// = 1;	// PCM
	unsigned short NumChannels;//;
	unsigned int SampleRate;//;
	unsigned int ByteRate;// 512k * 3 *1ch;
	unsigned short BlockAlign;// 3(24b) * 1ch;
	unsigned short BitsPerSample;// = 24;
	char Subchunk2ID[4];// = {'d','a','t','a'};
	unsigned int Subchunk2Size;// = FILE_BYTES_PER_SMP * NCH * SRATE_DECI * 60 * MINUTES_PER_FILE;
	MonoWavHeader()
	{
		ChunkID[0] = 'R'; ChunkID[1] = 'I'; ChunkID[2] = 'F'; ChunkID[3] = 'F';
		ChunkSize = 36;// + 92160000; // + FILE_BYTES_PER_SMP * NCH * SRATE_DECI * 60 * MINUTES_PER_FILE;
		Format[0] = 'W'; Format[1] = 'A'; Format[2] = 'V'; Format[3] = 'E';
		Subchunk1ID[0] = 'f'; Subchunk1ID[1] = 'm'; Subchunk1ID[2] = 't'; Subchunk1ID[3] = ' ';
		Subchunk1Size = 16;
		AudioFormat = 1;
		NumChannels = 2;
		SampleRate = 16000;
		ByteRate = 16000 * 3 * 2;
		BlockAlign = 6;
		BitsPerSample = 24;
		Subchunk2ID[0] = 'd'; Subchunk2ID[1] = 'a'; Subchunk2ID[2] = 't'; Subchunk2ID[3] = 'a';
		Subchunk2Size = 0;// + 92160000; //FILE_BYTES_PER_SMP * NCH * SRATE_DECI * 60 * MINUTES_PER_FILE;
	}
};
#pragma pack(pop)
MonoWavHeader wavHeader;

FIL fil0, fil1;		/* File object */


static u48 DatQueueCh0[SMP_PER_SECOND + 8], DatQueueCh1[SMP_PER_SECOND + 8];

void SDStoreData(u8 *DatCh0ptr, u8 *DatCh1ptr, u32 size)
{
	int yy, mm, dd, hh, min, ss;
	yy = DateTime.YearFrom2000+2000;
	mm = DateTime.Month;
	dd = DateTime.Day;
	hh = DateTime.Hour;
	min = DateTime.Minute;
	ss = DateTime.Second;
	static u32 dataSize = 0;
	static u32 chunkSize = 36;
	static int newfile = 0;
	static int newboot = 1;
	FRESULT res;
	static int yyl = -100, mml = -100, ddl = -100, hhl = -100, minl = -100;//, ssl = 0;//last time record
	static char str[100];
	if (yyl != yy || mml != mm || ddl != dd)
	{
		sprintf(str, "%04d%02d%02d", yy, mm, dd);
		yyl = yy;
		mml = mm;
		ddl = dd;
		res = f_mkdir(str);
		newfile = 1;
		//configASSERT(res == FR_OK || res == FR_EXIST);
	}
	if (hh != hhl || min >= minl + 10)
	{
		sprintf(str+8, "/%02d%02d%02d", hh, min, ss);
		hhl = hh;
		minl = min;
		newfile = 1;
		//ssl = ss;
	}
	if (newfile)
	{
		newfile = 0;
		if (!newboot)
		{
			res = f_sync(&fil0);
			res = f_sync(&fil1);
			res = f_close(&fil0);
			res = f_close(&fil1);
		}

		sprintf(str+15, "-Ch1.wav");
		res = f_open(&fil0, str, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
		sprintf(str+15, "-Ch2.wav");
		res = f_open(&fil1, str, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);

		dataSize = size;
		chunkSize = dataSize + 36;

		XGpio_DiscreteClear(&Gpio_reset_fifo, 2, 0x1);	//enlight led1
		//xil_printf("create ch1\n\r");
		//configASSERT(res == FR_OK);
		res = f_lseek(&fil0, 0);
		res = f_write(&fil0, (void *)&wavHeader, sizeof(MonoWavHeader), NULL);
		//configASSERT(res == FR_OK);
		res = f_write(&fil0, DatCh0ptr, size, NULL);
		//configASSERT(res == FR_OK);
		res = f_lseek(&fil0, (int)&(((MonoWavHeader *)0)->ChunkSize));
		//configASSERT(res == FR_OK);
		res = f_write(&fil0, &chunkSize, 4, NULL);
		//configASSERT(res == FR_OK);
		res = f_lseek(&fil0, (int)&(((MonoWavHeader *)0)->Subchunk2Size));
		//configASSERT(res == FR_OK);
		res = f_write(&fil0, &dataSize, 4, NULL);
		res = f_sync(&fil0);
		XGpio_DiscreteSet(&Gpio_reset_fifo, 2, 0x1);	//dislight led1

		XGpio_DiscreteClear(&Gpio_reset_fifo, 2, 0x2);	//enlight led2
		//xil_printf("create ch2\n\r");
		//configASSERT(res == FR_OK);
		res = f_lseek(&fil1, 0);
		res = f_write(&fil1, (void *)&wavHeader, sizeof(MonoWavHeader), NULL);
		//configASSERT(res == FR_OK);
		res = f_write(&fil1, DatCh1ptr, size, NULL);
		//configASSERT(res == FR_OK);
		res = f_lseek(&fil1, (int)&(((MonoWavHeader *)0)->ChunkSize));
		//configASSERT(res == FR_OK);
		res = f_write(&fil1, &chunkSize, 4, NULL);
		//configASSERT(res == FR_OK);
		res = f_lseek(&fil1, (int)&(((MonoWavHeader *)0)->Subchunk2Size));
		//configASSERT(res == FR_OK);
		res = f_write(&fil1, &dataSize, 4, NULL);
		res = f_sync(&fil1);
		XGpio_DiscreteSet(&Gpio_reset_fifo, 2, 0x2);	//dislight led2
	}
	else
	{
		dataSize += size;
		chunkSize = dataSize + 36;

		XGpio_DiscreteClear(&Gpio_reset_fifo, 2, 0x1);	//enlight led1
		//xil_printf("append ch1\n\r");
		res = f_lseek(&fil0, f_size(&fil0));
		//configASSERT(res == FR_OK);
		res = f_write(&fil0, DatCh0ptr, size, NULL);
		//configASSERT(res == FR_OK);
		res = f_lseek(&fil0, (int)&(((MonoWavHeader *)0)->Subchunk2Size));
		//configASSERT(res == FR_OK);
		res = f_lseek(&fil0, (int)&(((MonoWavHeader *)0)->ChunkSize));
		//configASSERT(res == FR_OK);
		res = f_write(&fil0, &chunkSize, 4, NULL);
		res = f_lseek(&fil0, (int)&(((MonoWavHeader *)0)->Subchunk2Size));
		//configASSERT(res == FR_OK);
		res = f_write(&fil0, &dataSize, 4, NULL);
		res = f_sync(&fil0);
		XGpio_DiscreteSet(&Gpio_reset_fifo, 2, 0x1);	//dislight led1

		XGpio_DiscreteClear(&Gpio_reset_fifo, 2, 0x2);	//enlight led2
		//xil_printf("append ch2\n\r");
		res = f_lseek(&fil1, f_size(&fil1));
		//configASSERT(res == FR_OK);
		res = f_write(&fil1, DatCh1ptr, size, NULL);
		//configASSERT(res == FR_OK);
		res = f_lseek(&fil1, (int)&(((MonoWavHeader *)0)->Subchunk2Size));
		//configASSERT(res == FR_OK);
		res = f_lseek(&fil1, (int)&(((MonoWavHeader *)0)->ChunkSize));
		//configASSERT(res == FR_OK);
		res = f_write(&fil1, &chunkSize, 4, NULL);
		res = f_lseek(&fil1, (int)&(((MonoWavHeader *)0)->Subchunk2Size));
		//configASSERT(res == FR_OK);
		res = f_write(&fil1, &dataSize, 4, NULL);
		res = f_sync(&fil1);
		XGpio_DiscreteSet(&Gpio_reset_fifo, 2, 0x2);	//dislight led2
	}
	newboot = 0;
	//xil_printf("%d   %d   %d \n\r", SdInstance[0].BusSpeed, SdInstance[0].BusWidth, SdInstance[0].Config.InputClockHz);
}


void tskSD(void *arg)
{
	static int cntsec = 0;
	while(1)
	{
		xSemaphoreTake(SemQueueCh0, portMAX_DELAY);
		xil_printf("sec=%d\n\r", cntsec );
		QueueCh0.DeBlock(DatQueueCh0 , SMP_PER_SECOND, 0, 0);
		QueueCh1.DeBlock(DatQueueCh1 , SMP_PER_SECOND, 0, 0);
		SDStoreData( (u8*)DatQueueCh0, (u8*)DatQueueCh1, SMP_PER_SECOND * BYTE_PER_SMP * 2);
		if (++cntsec == 60) cntsec = 0;

	}
}

void InitSD()
{
	memset(DatQueueCh0, 0, SMP_PER_SECOND * 6);
	memset(DatQueueCh1, 0, SMP_PER_SECOND * 6);

	static FATFS fatfs;
	FRESULT res;
	//char str[50];
	f_mount(&fatfs, "0:/", 0);
	/*while(1)
	{
		sprintf(str, "20000000/00%02d00-Ch1.wav",(int)DateTime.Minute);
		res = f_open(&fil0, str, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
		if (res == FR_OK)
		{
			DateTime.Minute++;
			f_close(&fil0);
		}
		else break;

	}*/
	xTaskCreate(tskSD, "tskSD", tskSDStkSize, NULL, tskSDPrio, &tskSDHandle);
}


void ExitSD()
{

}


void SDSpeedTest()
{
	static FATFS fatfs;
	FIL fil;
	char str[200];
	FRESULT res1, res2, res3 ;
	u32 size = 1024*1024*32, done = 0;  //32MB
	u8 array[size];
	static XTime t1, t2, t3, t4;
	res1 = f_mount(&fatfs, "0:/", 0);
	char strf[20];
	int cnt = 0;
	while(1){
		//f_unlink(strf);
		sprintf(strf, "test%d", cnt++);
		res1 = f_open(&fil, strf, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
		XTime_GetTime(&t1);
		memset((void *)array, 0, size);
		XTime_GetTime(&t2);
		res2 = f_write(&fil, (void *)array, size, (UINT *)&done);
		XTime_GetTime(&t3);
		res3 = f_sync(&fil);
		f_close(&fil);
		XTime_GetTime(&t4);
		//xil_printf("%d  %d  %d \n\r", res1, res2, res3);
		sprintf(str, "t1:  %f   t2: %f   t3: %f    t4: %f\n\rtime1: %f  time2: %f  time3: %f\n\r", (float)(t1),(float)(t2), (float)t3, (float)(t4),
				((float)(t2)-(float)(t1))/(XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ /2),
				((float)(t3)-(float)(t2))/(XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ /2),
				((float)(t4)-(float)(t3))/(XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ /2));
		xil_printf("%s", str);
		sleep_A9(1);
	}

}
