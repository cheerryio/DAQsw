#include <FreeRTOS.h>
#include <semphr.h>
#include <xparameters.h>
#include <stdio.h>
#include "../TskSD/TskSD.h"

extern u8 databuf[CHANNEL_NUM][DATABUF_NUM][DMA_TRANS_SIZE*4] __attribute__((aligned(64)));
extern SemaphoreHandle_t semDataBufWr[CHANNEL_NUM];
extern SemaphoreHandle_t semDataBufRd[CHANNEL_NUM];
extern unsigned int databuf_wr_p[CHANNEL_NUM];

void tskDataInit();
