#include "TskData.h"
#include <xparameters.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "xil_io.h"
#include "../common.h"

using namespace std;

extern XilIntc* intc;
extern XilAxiDma* axi_dma;

const int tskDataPrio = 2;
const int tskDataStkSize = 16384;
static TaskHandle_t tskDataHandle;

static SemaphoreHandle_t semFifoEmpty;
static SemaphoreHandle_t semDma;
extern SemaphoreHandle_t semDataBufWr;
extern SemaphoreHandle_t semDataBufRd;
unsigned int databuf_rd_p=0;

extern XScuGic xInterruptController;

extern u16 databuf[DATA_BUF_NUM][DATA_BUF_SIZE];

/**
 * @file TskData. cc
 * 
 * fifo 中断处理函数
 * 此时应该指示dma可以开始搬运数据
 * 
 * @param CallbackRef not used
 * @return
 */
void fifo_ISR(void* CallbackRef)
{
    static BaseType_t wake=pdTRUE;
    XScuGic_Disable(&xInterruptController,62U);
    xSemaphoreGiveFromISR(semFifoEmpty,&wake);
    portYIELD_FROM_ISR(wake);
}

/**
 * @file TskData. cc
 * 
 * dma完成中断函数
 * 当该中断函数触发时，代表一次dma传送完成
 * 
 * @param dmaps_channel
 * @param DmaCmd
 * @param CallbackRef
 * @return
 */
static void axi_dma_mm2s_ISR(void *CallbackRef){
    static BaseType_t wake=pdFALSE;
	u32 IrqStatus;
	IrqStatus = XAxiDma_IntrGetIrq(axi_dma->returnXAxiDma(), XAXIDMA_DMA_TO_DEVICE);
	XAxiDma_IntrAckIrq(axi_dma->returnXAxiDma(), IrqStatus, XAXIDMA_DMA_TO_DEVICE);
	if(!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)){
		return;
	}
	if((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)){
		XAxiDma_Reset(axi_dma->returnXAxiDma());
		return;
	}
	if((IrqStatus & XAXIDMA_IRQ_IOC_MASK)){
        xSemaphoreGiveFromISR(semDma,&wake);
        portYIELD_FROM_ISR(wake);
	}
    XScuGic_Enable(&xInterruptController,62U);
}

static void tskData(void* pvParameters){
    if(DEBUG)
        printf("in data task\n\r");
    u16* p;
    int ret;
    while(1){
        if(xSemaphoreTake(semDataBufRd,portMAX_DELAY)){
            if(DEBUG)
                printf("sending data to device\n\r");
            p=databuf[databuf_rd_p];
            databuf_rd_p=databuf_rd_p==DATA_BUF_NUM-1?0:databuf_rd_p+1;
            for(int i=0;i<DATA_BUF_SIZE/DMA_TRANS_SIZE;i++){
                if(xSemaphoreTake(semFifoEmpty,portMAX_DELAY)){
                    // dma transfer
                    //Xil_DCacheFlushRange((UINTPTR)p, DMA_TRANS_SIZE*sizeof(u16));
                    ret=axi_dma->DmaSimpleTransfer((UINTPTR)p, DMA_TRANS_SIZE*sizeof(u16), XAXIDMA_DMA_TO_DEVICE);
                    Xil_DCacheInvalidateRange((UINTPTR)p, DMA_TRANS_SIZE*sizeof(u16));
                    p=p+DMA_TRANS_SIZE;
                    xSemaphoreTake(semDma,portMAX_DELAY);   // blocking wait dma for finishing
                }
            }
            xSemaphoreGive(semDataBufWr);
        }
    }
    
    vTaskDelay(NULL);
}

void tskDataInit()
{
    int ret;

    semFifoEmpty=xSemaphoreCreateBinary();
    configASSERT(semFifoEmpty);

    semDma=xSemaphoreCreateBinary();
    configASSERT(semDma);

    // register interrupt
    //intc->InitChannelInt(62U, fifo_ISR, (void *)NULL);
    XScuGic_Connect(&xInterruptController,62U,(Xil_ExceptionHandler)fifo_ISR,(void*)NULL);
    XScuGic_Enable(&xInterruptController,62U);

    axi_dma->DmaReset();
    XScuGic_Connect(&xInterruptController,61U,(Xil_ExceptionHandler)axi_dma_mm2s_ISR,(void*)axi_dma);
    XScuGic_Enable(&xInterruptController,61U);
	//intc->InitChannelInt(XPAR_FABRIC_AXIDMA_0_VEC_ID, axi_dma_mm2s_ISR, (void *)axi_dma);
	XAxiDma_IntrEnable(axi_dma->returnXAxiDma(), XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

    ret=xTaskCreate(tskData,"TskData",1024,NULL,tskDataPrio,&tskDataHandle);
    configASSERT(ret==pdPASS);
}
