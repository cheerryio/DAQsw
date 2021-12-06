#include "TskData.h"
#include <xparameters.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "xil_io.h"
#include "../common.h"

using namespace std;

extern XilGpio* gpio_adc_ctrl;
extern XilAxiDma* axi_dma;

const int tskDataPrio = 2;
const int tskDataStkSize = 1024;
static TaskHandle_t tskDataHandle;

static SemaphoreHandle_t semFifoFill;
static SemaphoreHandle_t semDma;
SemaphoreHandle_t semDataBufWr;
SemaphoreHandle_t semDataBufRd;
unsigned int databuf_wr_p=0;

SemaphoreHandle_t semDataReady;

extern XScuGic xInterruptController;

int databuf[DATA_BUF_NUM][DATA_BUF_SIZE] __attribute__((aligned(32))) ;

volatile bool has_data=false;

/**
 * @file TskData. cc
 * 
 * dma完成中断函数
 * 当该中断函数触发时，代表一次dma传送完成
 * 
 * @param CallbackRef
 * @return
 */
static void axi_dma_ISR(void *CallbackRef)
{
    if(DEBUG)
        printf("in axi dma ISR\n\r");
	int msk;
    BaseType_t wake = 0;
    XilAxiDma *dma = (XilAxiDma *)CallbackRef;
    msk = XAxiDma_IntrGetIrq(dma->returnXAxiDma(), XAXIDMA_DEVICE_TO_DMA);
	if(msk & XAXIDMA_IRQ_IOC_MASK)
	{
	    int ret = xSemaphoreGiveFromISR(semDma, &wake);
	    
	    portYIELD_FROM_ISR(wake);
	}
	XAxiDma_IntrAckIrq(dma->returnXAxiDma(), XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
}

static void tskData(void* pvParameters){
    printf("in data task\n\r");
    int* p;
    int sts;

    xSemaphoreTake(semDataReady,portMAX_DELAY); // waiting for network connection
    for(int i=0;i<4;i++){
        do{
            sts=axi_dma->DmaSimpleTransfer((UINTPTR)databuf[0],DMA_TRANS_SIZE*sizeof(int),XAXIDMA_DEVICE_TO_DMA);
        }while(sts!=XST_SUCCESS);
        xSemaphoreTake(semDma,portMAX_DELAY);   // blocking wait dma for finishing
    }

    while(1){
        if(xSemaphoreTake(semDataBufWr,portMAX_DELAY)){
            p=databuf[databuf_wr_p];
            for(int i=0;i<DATA_BUF_SIZE/DMA_TRANS_SIZE;i++){
                // dma transfer
                do{
                    sts=axi_dma->DmaSimpleTransfer((UINTPTR)p,DMA_TRANS_SIZE*sizeof(int),XAXIDMA_DEVICE_TO_DMA);
                }while(sts!=XST_SUCCESS);
                xSemaphoreTake(semDma,portMAX_DELAY);   // blocking wait dma for finishing
                p=p+DMA_TRANS_SIZE;
            }
            Xil_DCacheInvalidateRange((UINTPTR)databuf[databuf_wr_p],DATA_BUF_SIZE*sizeof(int));
            databuf_wr_p=databuf_wr_p==DATA_BUF_NUM-1?0:databuf_wr_p+1;
            xSemaphoreGive(semDataBufRd);   // get one buffer ready for reading
        }
    }
    
    vTaskDelay(NULL);
}

void tskDataInit()
{
    int ret;

    semFifoFill=xSemaphoreCreateBinary();
    configASSERT(semFifoFill);

    semDma=xSemaphoreCreateBinary();
    configASSERT(semDma);

    semDataBufWr=xSemaphoreCreateCounting(DATA_BUF_NUM,DATA_BUF_NUM);
    configASSERT(semDataBufWr);

    semDataBufRd=xSemaphoreCreateCounting(DATA_BUF_NUM,0);
    configASSERT(semDataBufRd);

    semDataReady=xSemaphoreCreateBinary();
    configASSERT(semDataReady);

	XAxiDma_Reset(axi_dma->returnXAxiDma());
	while(!XAxiDma_ResetIsDone(axi_dma->returnXAxiDma()));

	XScuGic_Connect(&xInterruptController,XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR,(XInterruptHandler)axi_dma_ISR,(void *)axi_dma);
	XScuGic_Enable(&xInterruptController,XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR);
	XAxiDma_IntrEnable(axi_dma->returnXAxiDma(), XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);

    ret=xTaskCreate(tskData,"TskData",tskDataStkSize,NULL,tskDataPrio,&tskDataHandle);
    configASSERT(ret==pdPASS);
}
