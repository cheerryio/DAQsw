#include <FreeRTOS.h>
#include <semphr.h>
#include <xparameters.h>
#include <stdio.h>

#define DATA_ADDR XPAR_AXI_FIFO_IF_TOP_BASEADDR
#define DATABUF_SIZE 256
#define FIFO_THRESHOLD DATABUF_SIZE

void tskDataInit();
