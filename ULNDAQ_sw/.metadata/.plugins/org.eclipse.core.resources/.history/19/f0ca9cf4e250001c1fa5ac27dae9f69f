#include "TskData.h"
#include <xparameters.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <xil_cache.h>
#include "xil_io.h"
#include "../common.h"
#include "Queue.h"

using namespace std;

const int tskDataPrio = 9;
const int tskDataStkSize = 16384;
static TaskHandle_t tskDataHandle;

SemaphoreHandle_t semFifoFill0;
SemaphoreHandle_t semDma[CHANNEL_NUM];

u8 databuf[CHANNEL_NUM][DATABUF_NUM][DMA_TRANS_SIZE*4] __attribute__((aligned(64)));
SemaphoreHandle_t semDataBufWr[CHANNEL_NUM];
SemaphoreHandle_t semDataBufRd[CHANNEL_NUM];
unsigned int databuf_wr_p[CHANNEL_NUM]={0};


extern XScuGic xInterruptController;
/**
 * @file TskData. cc
 * 
 * fifo 中断处理函数
 * 当该中断函数触发时，代表硬件fifo中存储的数据超过了256
 * 此时应该指示dma可以开始搬运数据
 * 
 * @param CallbackRef not used
 * @return
 */
void fifo_ISR0(void* CallbackRef)
{
    if(DEBUG)
        printf("in fifo ISR0\n\r");
    static BaseType_t wake=pdTRUE;
    XScuGic_Disable(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_0_INTR);
    xSemaphoreGiveFromISR(semFifoFill0,&wake);
    portYIELD_FROM_ISR(wake);
}

void dma_ISR0(void* CallbackRef)
{
    if(DEBUG)
        printf("in dma ISR0\n\r");
    static BaseType_t wake=pdTRUE;
    Xil_Out32(0x40000004,1<<0);
    xSemaphoreGiveFromISR(semDma[0],&wake);
    portYIELD_FROM_ISR(wake);
}

void dma_ISR1(void* CallbackRef)
{
    if(DEBUG)
        printf("in dma ISR1\n\r");
    static BaseType_t wake=pdTRUE;
    Xil_Out32(0x40000004,1<<1);
    xSemaphoreGiveFromISR(semDma[1],&wake);
    portYIELD_FROM_ISR(wake);
}

void dma_ISR2(void* CallbackRef)
{
    if(DEBUG)
        printf("in dma ISR2\n\r");
    static BaseType_t wake=pdTRUE;
    Xil_Out32(0x40000004,1<<2);
    xSemaphoreGiveFromISR(semDma[2],&wake);
    portYIELD_FROM_ISR(wake);
}

void dma_ISR3(void* CallbackRef)
{
    if(DEBUG)
        printf("in dma ISR3\n\r");
    static BaseType_t wake=pdTRUE;
    Xil_Out32(0x40000004,1<<3);
    xSemaphoreGiveFromISR(semDma[3],&wake);
    portYIELD_FROM_ISR(wake);
}

void dma_ISR4(void* CallbackRef)
{
    if(DEBUG)
        printf("in dma ISR4\n\r");
    static BaseType_t wake=pdTRUE;
    Xil_Out32(0x40000004,1<<4);
    xSemaphoreGiveFromISR(semDma[4],&wake);
    portYIELD_FROM_ISR(wake);
}

void dma_ISR5(void* CallbackRef)
{
    if(DEBUG)
        printf("in dma ISR5\n\r");
    static BaseType_t wake=pdTRUE;
    Xil_Out32(0x40000004,1<<5);
    xSemaphoreGiveFromISR(semDma[5],&wake);
    portYIELD_FROM_ISR(wake);
}

void dma_ISR6(void* CallbackRef)
{
    if(DEBUG)
        printf("in dma ISR6\n\r");
    static BaseType_t wake=pdTRUE;
    Xil_Out32(0x40000004,1<<6);
    xSemaphoreGiveFromISR(semDma[6],&wake);
    portYIELD_FROM_ISR(wake);
}

void dma_ISR7(void* CallbackRef)
{
    if(DEBUG)
        printf("in dma ISR7\n\r");
    static BaseType_t wake=pdTRUE;
    Xil_Out32(0x40000004,1<<7);
    xSemaphoreGiveFromISR(semDma[7],&wake);
    portYIELD_FROM_ISR(wake);
}

static void tskData(void *pvParameters)
{
	if(DEBUG)
		printf("in task data\n\r");
    int data;
    xSemaphoreTake(semSDReady,portMAX_DELAY);
    Xil_Out32(0x40002000,3);
    while(1){
        xSemaphoreTake(semFifoFill0,portMAX_DELAY);
        for(int i=0;i<CHANNEL_NUM;i++){
            xSemaphoreTake(semDataBufWr[i],portMAX_DELAY);
            Xil_Out32(0x40000000+8,(u32)databuf[i][databuf_wr_p[i]]);
            Xil_Out32(0x40000000+12,DMA_TRANS_SIZE);
            Xil_Out32(0x40000000,1<<i);
        }
        for(int i=0;i<CHANNEL_NUM;i++){
            xSemaphoreTake(semDma[i],portMAX_DELAY);
            xSemaphoreGive(semDataBufRd[i]);
            databuf_wr_p[i]=databuf_wr_p[i]==DATABUF_NUM-1?0:databuf_wr_p[i]+1;
        }
        XScuGic_Enable(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_0_INTR);
    }

    vTaskDelete(NULL);
}

void tskDataInit()
{
    int ret;

    semFifoFill0=xSemaphoreCreateBinary();
    configASSERT(semFifoFill0);
    
    for(int i=0;i<CHANNEL_NUM;i++){
        semDma[i]=xSemaphoreCreateBinary();
        configASSERT(semDma[i]);
        semDataBufWr[i]=xSemaphoreCreateCounting(DATABUF_NUM,DATABUF_NUM);
        configASSERT(semDataBufWr[i]);
        semDataBufRd[i]=xSemaphoreCreateCounting(DATABUF_NUM,0);
        configASSERT(semDataBufRd[i]);
    }

    XScuGic_Connect(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_0_INTR,(Xil_ExceptionHandler)fifo_ISR0,NULL);
    XScuGic_Enable(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_0_INTR);

#if CHANNEL_NUM > 0
    XScuGic_Connect(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_1_INTR,(Xil_ExceptionHandler)dma_ISR0,NULL);
    XScuGic_Enable(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_1_INTR);
#endif
#if CHANNEL_NUM > 1
    XScuGic_Connect(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_2_INTR,(Xil_ExceptionHandler)dma_ISR1,NULL);
    XScuGic_Enable(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_2_INTR);
#endif
#if CHANNEL_NUM > 2
    XScuGic_Connect(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_3_INTR,(Xil_ExceptionHandler)dma_ISR2,NULL);
    XScuGic_Enable(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_3_INTR);
#endif
#if CHANNEL_NUM > 3
    XScuGic_Connect(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_4_INTR,(Xil_ExceptionHandler)dma_ISR3,NULL);
    XScuGic_Enable(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_4_INTR);
#endif
#if CHANNEL_NUM > 4
    XScuGic_Connect(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_5_INTR,(Xil_ExceptionHandler)dma_ISR4,NULL);
    XScuGic_Enable(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_5_INTR);
#endif
#if CHANNEL_NUM > 5
    XScuGic_Connect(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_6_INTR,(Xil_ExceptionHandler)dma_ISR5,NULL);
    XScuGic_Enable(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_6_INTR);
#endif
#if CHANNEL_NUM > 6
    XScuGic_Connect(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_7_INTR,(Xil_ExceptionHandler)dma_ISR6,NULL);
    XScuGic_Enable(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_7_INTR);
#endif
#if CHANNEL_NUM > 7
    XScuGic_Connect(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_8_INTR,(Xil_ExceptionHandler)dma_ISR7,NULL);
    XScuGic_Enable(&xInterruptController,XPAR_FABRIC_ULNDAQ_IRQS_8_INTR);
#endif

    ret=xTaskCreate(tskData,"TskData",tskDataStkSize,NULL,tskDataPrio,&tskDataHandle);
    configASSERT(ret==pdPASS);
}
