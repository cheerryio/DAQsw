#ifndef __TSKDATA_H__
#define __TSKDATA_H__

#include <FreeRTOS.h>
#include <semphr.h>
#include <xparameters.h>
#include <stdio.h>

#define DATA_ADDR XPAR_AXI4LITE_STREAM_FIFO_BASEADDR
#define FIFOCNT_ADDR XPAR_RD_CNT_BASEADDR

void tskDataInit();

#endif