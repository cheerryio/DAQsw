#include "TskData.h"
#include <xparameters.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <ff.h>
#include <xsdps.h>
#include "../common.h"
#include "../TskSD/SD.h"
#include "../TskHif/TskHif.h"

using namespace std;

extern XilIntc* intc;
extern XilGpio* gpio_adc_en;
extern XilAxiDma* axi_dma;

const int tskDataPrio = 8;
const int tskDataStkSize = 16384;
static TaskHandle_t tskDataHandle;

static SemaphoreHandle_t semDma;

extern XScuGic xInterruptController;
extern QueueHandle_t tskHifMsgQ;
extern SemaphoreHandle_t semSDReady;

u8 databuf[DATABUF_NUM][DMA_TRANS_SIZE*3] __attribute__((aligned(128)));
SemaphoreHandle_t semDataBufWr;
SemaphoreHandle_t semDataBufRd;
unsigned int databuf_wr_p=0;

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
    if(DEBUG)
        printf("in data task\n\r");
    int sts,ret;
    u8* p;
    xSemaphoreTake(semSDReady,portMAX_DELAY);
    gpio_adc_en->XilGpioDiscreteWrite(1,1);
    while(1){
        xSemaphoreTake(semDataBufWr,portMAX_DELAY);
        p=databuf[databuf_wr_p];
        // dma transfer
        do{
            sts=axi_dma->DmaSimpleTransfer((UINTPTR)p,DMA_TRANS_SIZE*3,XAXIDMA_DEVICE_TO_DMA);
        }while(sts != XST_SUCCESS);
        xSemaphoreTake(semDma,portMAX_DELAY);   // blocking wait dma for finishing
        xSemaphoreGive(semDataBufRd);
        databuf_wr_p=databuf_wr_p==DATABUF_NUM-1?0:databuf_wr_p+1;
    }
    
    vTaskDelay(NULL);
}

void tskDataInit()
{
    int ret;

    semDma=xSemaphoreCreateBinary();
    configASSERT(semDma);

    semDataBufWr=xSemaphoreCreateCounting(DATABUF_NUM,DATABUF_NUM);
    configASSERT(semDataBufWr);

    semDataBufRd=xSemaphoreCreateCounting(DATABUF_NUM,0);
    configASSERT(semDataBufRd);

	XAxiDma_Reset(axi_dma->returnXAxiDma());
	while(!XAxiDma_ResetIsDone(axi_dma->returnXAxiDma()));

	intc->InitChannelInt(XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR, (XInterruptHandler)&axi_dma_ISR, (void *)axi_dma);
	XAxiDma_IntrEnable(axi_dma->returnXAxiDma(), XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
    
    ret=xTaskCreate(tskData,"TskData",1024,NULL,tskDataPrio,&tskDataHandle);
    configASSERT(ret==pdPASS);
}
