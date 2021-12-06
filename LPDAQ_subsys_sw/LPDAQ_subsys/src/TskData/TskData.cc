#include "TskData.h"
#include <xparameters.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "xil_io.h"
#include "../common.h"
#include "Queue.h"

using namespace std;

extern XilGpio* gpio_adc_ctrl;
Queue<int,true> dataQ(512000);

const int tskDataPrio = 9;
const int tskDataStkSize = 16384;
static TaskHandle_t tskDataHandle;

SemaphoreHandle_t semFifoFill;
SemaphoreHandle_t semDma;

int databuf[DATABUF_SIZE] __attribute__ ((aligned (64)));

extern XScuGic xInterruptController;
extern XilPsDma* ps_dma;
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
void fifo_ISR(void* CallbackRef)
{
    if(DEBUG)
        printf("in axi4l fifo if ISR\n\r");
    static BaseType_t wake=pdTRUE;
    XScuGic_Disable(&xInterruptController,XPAR_FABRIC_AXI_FIFO_IF_TOP_PROG_FULL_INTR_INTR);
    xSemaphoreGiveFromISR(semFifoFill,&wake);
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
static void dmaps_ISR(unsigned int dmaps_channel, XDmaPs_Cmd *DmaCmd, void *CallbackRef){
    if(DEBUG)
        printf("in dmaps ISR\n\r");
    static BaseType_t wake=pdTRUE;
    xSemaphoreGiveFromISR(semDma,&wake);
    portYIELD_FROM_ISR(wake);
}

static void tskData(void *pvParameters)
{
	if(DEBUG)
		printf("in task data\n\r");
    int data;
    gpio_adc_ctrl->XilGpioDiscreteWrite(2,1);
    gpio_adc_ctrl->XilGpioDiscreteWrite(1,1);
    while(1){
        xSemaphoreTake(semFifoFill,portMAX_DELAY);
        //ps_dma->DmaSimpleTransfer(4,16,0,4,16,1,DATA_ADDR,(u32)databuf,DATABUF_SIZE*sizeof(int));
        //xSemaphoreTake(semDma,portMAX_DELAY);
        //dataQ.EnBlock(databuf,DATABUF_SIZE,true);

        /*
        for(int i=0;i<256;i++){
            data=Xil_In32(DATA_ADDR);
            dataQ.En(data,true);
        }
        */
        printf("go to write file\n\r");
        SDWriteInt((int*)DATA_ADDR,256,"data.bin");
        printf("write file success\n\r");
        XScuGic_Enable(&xInterruptController,XPAR_FABRIC_AXI_FIFO_IF_TOP_PROG_FULL_INTR_INTR);

    }

    vTaskDelete(NULL);
}

void tskDataInit()
{
    int ret;

    semFifoFill=xSemaphoreCreateBinary();
    configASSERT(semFifoFill);

    semDma=xSemaphoreCreateBinary();
    configASSERT(semDma);

    XScuGic_Connect(&xInterruptController,XPAR_FABRIC_AXI_FIFO_IF_TOP_PROG_FULL_INTR_INTR,(Xil_ExceptionHandler)fifo_ISR,NULL);
    XScuGic_Enable(&xInterruptController,XPAR_FABRIC_AXI_FIFO_IF_TOP_PROG_FULL_INTR_INTR);

    XDmaPs_SetDoneHandler(ps_dma->returnPsDma(),0,(XDmaPsDoneHandler)dmaps_ISR,(void*)ps_dma);
    XScuGic_Connect(&xInterruptController,XPS_DMA0_INT_ID,(Xil_ExceptionHandler)XDmaPs_DoneISR_0,(void*)(ps_dma->returnPsDma()));
    XScuGic_Enable(&xInterruptController,XPS_DMA0_INT_ID);

    ret=xTaskCreate(tskData,"TskData",163840,NULL,tskDataPrio,&tskDataHandle);
    configASSERT(ret==pdPASS);
}
