/*
 * Empty C++ Application
 */

// freeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <timers.h>
// xilinx
#include <xil_printf.h>
#include <xparameters.h>
#include <mb_interface.h>
#include <xintc.h>
#include <sleep.h>
#include "MiniDmaFifo2Mem/MiniDmaFifo2Mem.h"
#include <xil_types.h>
#include <xstatus.h>

#include "xilffs/ff.h"
#include <stdio.h>
#include "IicMaster/IicMaster.h"
#include <xgpio.h>

#include<cstring>

#include <xuartlite.h>
#include "TskHif/TskHif.h"
#include "TskHif/PCF8563.h"

extern DateTime32 DateTime;

void dcmshowError()
{
	while(1)
	{
		dcmLedSet &= 0x5f; // 0b0101_1111  dcm all led turn on
		vTaskDelay(100);
		dcmLedSet |= 0xe0; // 0b1110_0000  dcm all led turn on
		vTaskDelay(100);
	}
}

const u32 bufLen = 1536000 * 5;	// must be a divisor of 1536000*60

XGpio Led;

__attribute__ ((aligned(64))) u8 data0[bufLen];
__attribute__ ((aligned(64))) u8 data1[bufLen];
u8 * volatile dataAvil = data0;

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
		NumChannels = 1;
		SampleRate = 512000;
		ByteRate = 512000 * 3;
		BlockAlign = 3;
		BitsPerSample = 24;
		Subchunk2ID[0] = 'd'; Subchunk2ID[1] = 'a'; Subchunk2ID[2] = 't'; Subchunk2ID[3] = 'a';
		Subchunk2Size = 0;// + 92160000; //FILE_BYTES_PER_SMP * NCH * SRATE_DECI * 60 * MINUTES_PER_FILE;
	}
};
#pragma pack(pop)
MonoWavHeader wavHeader;

/*

//#define YEAR	2018
//#define MONTH	7
//#define	DATE	13
//#define DAY		5
//#define HOUR	00
//#define MINUTE	54
//#define SECOND	00

//#define YEAR	2021
//#define MONTH	4
//#define	DATE	22
//#define DAY		4
//#define HOUR	23
//#define MINUTE	0
//#define SECOND	0

typedef union _DateTime
{
	u32 Value;
	struct
	{
		u32 DSecond 		: 5;
		u32 Minute			: 6;
		u32 Hour			: 5;
		u32 Date			: 5;
		u32 Month			: 4;
		u32 YearFrom1980	: 7;
	};
} DateTime;

volatile u32 dateTime =   ((u32)(2021U - 1980U) << 25U)	// initialize to June 18, 2018
						| ((u32)4 << 21)
						| ((u32)22 << 16)
						| ((u32)23 << 11)
						| ((u32)0 << 5)
						| ((u32)0 >> 1);
*/

XIntc Intc;

void InitIntc()
{
    int sts;

    sts = XIntc_Initialize(&Intc, XPAR_INTC_0_DEVICE_ID);
    configASSERT(sts == XST_SUCCESS);

    sts = XIntc_Start(&Intc, XIN_REAL_MODE);
    configASSERT(sts == XST_SUCCESS);

    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(
            XIL_EXCEPTION_ID_INT,
            (Xil_ExceptionHandler)XIntc_InterruptHandler,
            &Intc
    );
    Xil_ExceptionEnable();
}

void SetLed(u32 mask)
{
	XGpio_DiscreteSet(&Led, 1, mask & 0xff);
	XGpio_DiscreteClear(&Led, 1, mask & 0x300);
}
void ClrLed(u32 mask)
{
	XGpio_DiscreteClear(&Led, 1, mask & 0xff);
	XGpio_DiscreteSet(&Led, 1, mask & 0x300);
}

XUartLite uart0;
char txlen, txBuf[256];
char rxlen, rxBuf[256];
int uartOut(char * buf_str, u8 txlen)
{
	u8 sendN = 0;
	if((txlen - sendN) > 16)
	{
		do{
			while(true == XUartLite_IsSending(&uart0)); // wait send done;
			XUartLite_Send(&uart0, (u8 *)(buf_str+sendN), 16);
			sendN += 16;
		}while((txlen - sendN) > 16);
	}

	XUartLite_Send(&uart0, (u8 *)(buf_str+sendN), txlen - sendN);
	while(true == XUartLite_IsSending(&uart0)); // wait send done;

	return 0;
}

void u_printf(char *format, ...) {
	char buf_str[256];
	va_list v_args;

	memset(buf_str, 0, sizeof(buf_str));
	va_start(v_args, format);
	(void)vsnprintf((char *)&buf_str[0], (size_t) sizeof(buf_str), (char const *) format, v_args);
	va_end(v_args);

//	xSemaphoreTake(mutexPrintf, portMAX_DELAY);
	uartOut(buf_str, strnlen(buf_str, sizeof(buf_str)));
//	xSemaphoreGive(mutexPrintf);
}

void InitUart()
{
    int sts = XUartLite_Initialize (&uart0, XPAR_AXI_UARTLITE_0_DEVICE_ID);
    configASSERT(sts == XST_SUCCESS);
//    uartPrintf("uartlite start!\n\r");
    u_printf("UART Port BandRate 9600 Data 8bit Stop 1bit None Parity\r\n");
}


TaskHandle_t tskMasterHandle;
SemaphoreHandle_t semDmaFinish;
void dmaCallback(MiniDmaFifo2Mem::RegIntr intSts, u64 prevAddr, void *arg)
{
	configASSERT(!intSts.error);
	dataAvil = (u8 *)prevAddr;
	int ret;
	BaseType_t wake;
	ret = xSemaphoreGiveFromISR(semDmaFinish, &wake);
	configASSERT(ret == pdPASS || ret == errQUEUE_FULL);
}

volatile int idleCnt = 0;
void vApplicationIdleHook()
{
	idleCnt++;
}

__attribute__ ((aligned(64))) FATFS fs;
__attribute__ ((aligned(64))) FIL fil;        // File object
void initSDFAT()
{
	FRESULT fr;     // FatFs return code

	fr = f_mount(&fs, "", 1);
	if(fr != FR_OK)
	{
//		u_printf("TF Card Error !\r\n");
		printf("TF Card Error !\r\n");
		do
		{
			fr = f_mount(&fs, (const TCHAR *)(""), 1);
			dcmLedSet &= 0x5f; // 0b0001_1111  dcm blue red led turn on
			vTaskDelay(500);
			dcmLedSet |= 0xe0; // 0b1110_0000  dcm blue red led turn off
			vTaskDelay(500);
			tfCardSta = 0x01;
		}while(fr != FR_OK);

	}
	dcmLedSet &= 0x7f; // 0b0111_1111  dcm blue led turn on
	tfCardSta = 0x02; // tf card ok

	fr = f_open(&fil, "message.txt", FA_CREATE_ALWAYS | FA_WRITE);
	int l;
	l = f_puts("This is a data disk wrote by DAQ1245123 from Wuhan Duna(TM) Co. LTD.\r\n", &fil);
    configASSERT(l == 70);
	l = f_puts("Data are organized by 1 wave file per minute and 1 folder each power on,\r\n", &fil);
    configASSERT(l == 74);
	l = f_puts("the file name \"YYYYMMDD_hhmmss.WAV\" indicates time and the folder name \"YYYYMMDD\" indicates power on date.", &fil);
    configASSERT(l == 106);

	fr = f_close(&fil);
    configASSERT(fr == FR_OK);
}

char fn[64] = {'\0'};
static void tskMaster(void *param)
{
	initSDFAT();
//	if(0x01 == tfCardSta)dcmshowError();
//	uartPrintf("TF Card initial done\r\n");
//	printf("MicroSD Card FatFS initial done!\n\r");

	semDmaFinish = xSemaphoreCreateBinary();
	configASSERT(semDmaFinish);

	MiniDmaFifo2Mem dma(XPAR_PERIPHERALS_MINIDMAFIFO2MEM_0_BASEADDR);

	dma.SetAddress((u64)data0, (u64)data1);
	dma.SetFifoPentTh(448);
	MiniDmaFifo2Mem::RegControll ctrl;
	ctrl.Penetrate = true;
	ctrl.ContTrig = false;
	ctrl.FrameTrig = false;
	dma.SetControl(ctrl);	// set pent to discard existed fifo data
	vTaskDelay(1);			// wait fifo flush itself
	dma.RegisterCallback(
			&Intc,
			XPAR_PERIPHERALS_MICROBLAZE_0_AXI_INTC_PERIPHERALS_MINIDMAFIFO2MEM_0_INTR_INTR,
			dmaCallback,
			(void *)&dma
			);
	dma.SetTransferLen(bufLen / 4);

//	DateTime dt;
//	u8 cnt = 100;
//	do	// wait for time dsec changing
//	{
//		dt.Value = dateTime;
//		vTaskDelay(1);
//		cnt--;
//		if(cnt ==0 )break;
//	}
//	while(dt.Value == dateTime);
//	vTaskDelay(50);	// wait 0.5sec to anti time dsec aliasing

	ctrl.FrameTrig = true;
	dma.SetControl(ctrl);	// enable frame trig, this will start transfer automatically
	SetLed(0x202);

	FRESULT fr;     // FatFs return code
	UINT bw;
	u32 chunkSize;
	u32 dataSize = 0;
	DIR dir;
	int ret;
//	u32 channel_en, last_en;
	DateTime32 dcm;

	vTaskDelay(3000);	// wait DCM RTC init done
	dcm.Value = DateTime.Value;

	printf("RTC:%4u-%02u-%02u %02u:%02u:%02u\r\n",
				dcm.YearFrom2000+2000, dcm.Month, dcm.Day,
				dcm.Hour, dcm.Minute, dcm.Second);
//	vTaskDelay(100);

//	channel_en = XGpio_DiscreteRead(&gpio_CH_EN, 1);
//	if(last_en != channel_en)
//	{
//		last_en = channel_en;
//		channelSta = channel_en;
//		for(u8 i=0; i<CH_NUM; i++)
//		{
//			if(i<IEPE_NUM)
//				printf("IEPE CH%1d : %s\r\n", i%10+1, ((0x01<<i)&channel_en)? "ON":"OFF");
//			else
//				printf("DIFF CH%1d : %s\r\n", i%10+1-4, ((0x01<<i)&channel_en)? "ON":"OFF");
//		}
//	}
//	if( 0x00 == channel_en )// no channel_en, waiting channge
//	{
//		while( 0x00 == channel_en )
//		{
//			vTaskDelay(500);
//			channel_en = XGpio_DiscreteRead(&gpio_CH_EN, 1);
//			vTaskDelay(500);
//			dcm.Value = DateTime.Value;
//			printf("no channel select\r\n");
//		}
//	}
	u_printf("DAQ1245123 start recording\r\n");

	// create new folder per day
	sprintf(fn, "%04d%02d%02d", (int)(dcm.YearFrom2000 + 2000), (int)dcm.Month, (int)dcm.Day);
	fr = f_opendir(&dir, (const TCHAR *)fn);
	if(fr != FR_OK)fr = f_mkdir((const TCHAR *)fn);

//	dt.Value = dateTime;
//	sprintf(fn, "%04d%02d%02d", (int)(dt.YearFrom1980 + 1980), (int)dt.Month, (int)dt.Date);
//	fr = f_mkdir(fn);

	while(true)
	{
		sprintf(fn + 8, "/%04d%02d%02d_%02d%02d%02d.wav",
				(int)(dcm.YearFrom2000 + 2000), (int)dcm.Month, (int)dcm.Day,
				(int)dcm.Hour, (int)dcm.Minute, (int)dcm.Second);
		fr = f_open(&fil, fn, FA_CREATE_ALWAYS | FA_WRITE);
		configASSERT(fr == FR_OK);
		fr = f_write(&fil, (void *)&wavHeader, sizeof(MonoWavHeader), &bw);
		configASSERT(fr == FR_OK && bw == sizeof(MonoWavHeader));
		for(u32 sec = 0; sec < (1536000*60) / bufLen; sec++)	// 1min
		{
			for(int b = 0; b < 1; b++)
			{
				ret = xSemaphoreTake(semDmaFinish, portMAX_DELAY);
				configASSERT(ret == pdPASS);

				SetLed(0x101);
				fr = f_write(&fil, dataAvil, bufLen, &bw);
				configASSERT(fr == FR_OK && bw == bufLen);
				ClrLed(0x101);

				dataSize += bufLen;
			}
			chunkSize = dataSize + 36;

			SetLed(0x101);
			f_lseek(&fil, (int)&(((MonoWavHeader *)0)->ChunkSize));
			f_write(&fil, &chunkSize, 4, &bw);
			f_lseek(&fil, (int)&(((MonoWavHeader *)0)->Subchunk2Size));
			f_write(&fil, &dataSize, 4, &bw);
			f_lseek(&fil, file_size(&fil));
			f_sync(&fil);
			ClrLed(0x101);

		}
		fr = f_close(&fil);
	    configASSERT(fr == FR_OK);
//		dt.Value = dateTime;
//	    dcm.Value != DateTime.Value)dcm.Value = DateTime.Value;
	    dcm.Value = DateTime.Value;
	}

}

/*
char fn[64] = {'\0'};
static void tskMaster(void *param)
{
	initSDFAT();

//	printf("MicroSD Card FatFS initial done!\n\r");

	semDmaFinish = xSemaphoreCreateBinary();
	configASSERT(semDmaFinish);

	MiniDmaFifo2Mem dma(XPAR_PERIPHERALS_MINIDMAFIFO2MEM_0_BASEADDR);

	dma.SetAddress((u64)data0, (u64)data1);
	dma.SetFifoPentTh(448);
	MiniDmaFifo2Mem::RegControll ctrl;
	ctrl.Penetrate = true;
	ctrl.ContTrig = false;
	ctrl.FrameTrig = false;
	dma.SetControl(ctrl);	// set pent to discard existed fifo data
	vTaskDelay(1);			// wait fifo flush itself
	dma.RegisterCallback(
			&Intc,
			XPAR_PERIPHERALS_MICROBLAZE_0_AXI_INTC_PERIPHERALS_MINIDMAFIFO2MEM_0_INTR_INTR,
			dmaCallback,
			(void *)&dma
			);
	dma.SetTransferLen(bufLen / 4);

	DateTime dt;
	u8 cnt = 100;
	do	// wait for time dsec changing
	{
		dt.Value = dateTime;
		vTaskDelay(1);
		cnt--;
		if(cnt ==0 )break;
	}
	while(dt.Value == dateTime);
	vTaskDelay(50);	// wait 0.5sec to anti time dsec aliasing

	ctrl.FrameTrig = true;
	dma.SetControl(ctrl);	// enable frame trig, this will start transfer automatically
	SetLed(0x202);

	FRESULT fr;     // FatFs return code
	UINT bw;
	u32 chunkSize;
	u32 dataSize = 0;

	dt.Value = dateTime;
	sprintf(fn, "%04d%02d%02d", (int)(dt.YearFrom1980 + 1980), (int)dt.Month, (int)dt.Date);
	fr = f_mkdir(fn);

	int ret;
	while(true)
	{
		sprintf(fn + 8, "/%02d%02d%02d.wav", (int)dt.Hour, (int)dt.Minute, (int)(dt.DSecond * 2));
		fr = f_open(&fil, fn, FA_CREATE_ALWAYS | FA_WRITE);
		configASSERT(fr == FR_OK);
		fr = f_write(&fil, (void *)&wavHeader, sizeof(MonoWavHeader), &bw);
		configASSERT(fr == FR_OK && bw == sizeof(MonoWavHeader));
		for(u32 sec = 0; sec < (1536000*60) / bufLen; sec++)	// 1min
		{
			for(int b = 0; b < 1; b++)
			{
				ret = xSemaphoreTake(semDmaFinish, portMAX_DELAY);
				configASSERT(ret == pdPASS);

				SetLed(0x101);
				fr = f_write(&fil, dataAvil, bufLen, &bw);
				configASSERT(fr == FR_OK && bw == bufLen);
				ClrLed(0x101);

				dataSize += bufLen;
			}
			chunkSize = dataSize + 36;

			SetLed(0x101);
			f_lseek(&fil, (int)&(((MonoWavHeader *)0)->ChunkSize));
			f_write(&fil, &chunkSize, 4, &bw);
			f_lseek(&fil, (int)&(((MonoWavHeader *)0)->Subchunk2Size));
			f_write(&fil, &dataSize, 4, &bw);
			f_lseek(&fil, file_size(&fil));
			f_sync(&fil);
			ClrLed(0x101);

		}
		fr = f_close(&fil);
	    configASSERT(fr == FR_OK);
		dt.Value = dateTime;
	}

}


//#define SET_DATETIME

void setDs3231DateTime(IicMaster *iic, u16 yr, u8 mon, u8 dat, u8 day, u8 hr, u8 min, u8 sec)
{
	sec = sec % 10 | sec / 10 << 4;
	min = min % 10 | min / 10 << 4;
	hr	= hr  % 10 | hr  / 10 << 4;	// 24hr format
	day = day == 0 ? 7 : day;
	dat = dat % 10 | dat / 10 << 4;
	mon = mon % 10 | mon / 10 << 4 | (yr >= 2100 ? 0x80 : 0x00);
	yr  = yr  % 100;
	yr  = yr  % 10 | yr  / 10 << 4;
	iic->WriteRegPush(0xd0, 0, 7, sec, min, hr, day, dat, mon, yr);
	iic->WriteRegPop(7);
}

TaskHandle_t tskDateTimeHandle;
static void tskDateTime(void *arg)
{
	DateTime dt;
	u8 sec, min, hr, day, date, month, year;
	IicMaster iic(
			XPAR_PERIPHERALS_AXI4LITE_BUFFERED_II_0_S_AXI_BASEADDR,
			&Intc,
			XPAR_PERIPHERALS_MICROBLAZE_0_AXI_INTC_PERIPHERALS_AXI4LITE_BUFFERED_II_0_IRQ_INTR,
			NULL, NULL, NULL, NULL);

#ifdef SET_DATETIME
	setDs3231DateTime(&iic, YEAR, MONTH, DATE, DAY, HOUR, MINUTE, SECOND);
#endif
	iic.ReadRegPush(0xd0, 0, 7);
	vTaskDelay(10);
	while(true)
	{
		iic.ReadRegPop(7, &sec, &min, &hr, &day, &date, &month, &year);
		dt.DSecond = ((sec & 0xf) + 10 * (sec >> 4)) / 2;
		dt.Minute  =  (min & 0xf) + 10 * (min >> 4);
		dt.Hour	= hr & 0x40 ?
				(hr & 0x20 ? 12 : 0) + (hr & 0xf) + 10 * (hr >> 4 & 0x1) :
				(hr & 0xf) + 10 * (hr >> 4 & 0x3);
		dt.Date = (date & 0xf) + 10 * (date >> 4);
		dt.Month = (month & 0xf) + 10 * (month >> 4 & 0x1);
		dt.YearFrom1980 = (year & 0xf) + 10 * (year >> 4) + 100 * (month >> 7 & 0x1);
		dt.YearFrom1980 = dt.YearFrom1980 + 20;
		dateTime = dt.Value;
		if(dt.YearFrom1980 + 1980 < YEAR)
			setDs3231DateTime(&iic, YEAR, MONTH, DATE, DAY, HOUR, MINUTE, SECOND);
		iic.ReadRegPush(0xd0, 0, 7);
		vTaskDelay(10);
	}
}
*/

int main()
{
	microblaze_enable_icache();
	microblaze_enable_dcache();
	InitIntc();
	InitUart();

	int rtn = XGpio_Initialize(&Led, XPAR_PERIPHERALS_AXI_GPIO_LEDS_DEVICE_ID);
	configASSERT(rtn == XST_SUCCESS);
	ClrLed(0x3ff);

	tskHifInit(); // dcm task

	xTaskCreate(tskMaster, "TskMaster", 8192, NULL, 5, &tskMasterHandle);
//	xTaskCreate(tskDateTime, "TskDateTime", 512, NULL, 4, &tskDateTimeHandle);


	vTaskStartScheduler();

	while(1);

	return 0;
}
