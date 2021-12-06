#ifndef _TSKDATA_H_
#define _TSKDATA_H_

#include <FreeRTOS.h>
#include <semphr.h>
#include "../common.h"
#include "secCnt.h"
#include "ff.h"

#define DATA_BYTE_NUM 3
#define ADC_SAMPLE_RATE 450000
#define NUM_PER_DMA_DATA_OP 15000

#define TOTAL_DATA_BYTE NUM_PER_DMA_DATA_OP*DATA_BYTE_NUM
#define DATA_BUFFER_LEN TOTAL_DATA_BYTE
#define DATABUF_NUM 60
#define FFTBUF_NUM 20
#define TOTAL_FFT_BYTE 16384*DATA_BYTE_NUM

void readADCData(UINTPTR dstPtr, u32 len);
void test_acquisition_data();
void test_acquisition_data2(void *arg);
void test_acquisition_FFT();
void tskDataInit();

extern SemaphoreHandle_t semDataBufWr;
extern SemaphoreHandle_t semDataBufRd;
extern u8 databuf[DATABUF_NUM][TOTAL_DATA_BYTE] __attribute__((aligned(128)));
extern u8 fftbuf[FFTBUF_NUM][TOTAL_FFT_BYTE] __attribute__((aligned(128)));
#endif
