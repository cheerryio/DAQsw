/*
 * tskData.cc
 *
 *  Created on: Mar 7, 2021
 *      Author: WuXJ
 */
#include "tskData.h"

#include "ff.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "stdio.h"

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
//#include "FifoIf/FifoIf.h"
#include "Queue.h"
#include "common.h"
#include "sleep.h"
#include "xtime_l.h"
#include "xaxidma.h"
#include "xgpio.h"
#include "IQ_Demodulator.h"
TaskHandle_t tskDataHandle;
SemaphoreHandle_t SemFifoCh0, SemFifoCh1, SemQueueCh0, SemQueueCh1;



const int tskDataPrio = 7;
const int tskDataStkSize = 4096 * 16;

Queue<u48 , true> QueueCh0(SMP_PER_SECOND * 8);
Queue<u48 , true> QueueCh1(SMP_PER_SECOND * 8);
__attribute__((aligned(16))) static u64 temp0[SMP_PER_FRAME + 8];
__attribute__((aligned(16))) static u64 temp1[SMP_PER_FRAME + 8];

void FifoCh0Callback(void *arg)
{
	int ret;
    BaseType_t wake = 0;
    XAxiDma *dma = (XAxiDma *)arg;
    ret = xSemaphoreGiveFromISR(SemFifoCh0, &wake);
    configASSERT(ret == pdPASS || ret == errQUEUE_FULL);
	ret = XAxiDma_IntrGetIrq(dma, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrAckIrq(dma, ret, XAXIDMA_DEVICE_TO_DMA);
	configASSERT(ret & XAXIDMA_IRQ_ALL_MASK);
    portYIELD_FROM_ISR(wake);
}

void FifoCh1Callback(void *arg)
{
	int ret;
    BaseType_t wake = 0;
    XAxiDma *dma = (XAxiDma *)arg;
    ret = xSemaphoreGiveFromISR(SemFifoCh1, &wake);
    configASSERT(ret == pdPASS || ret == errQUEUE_FULL);
    ret = XAxiDma_IntrGetIrq(dma, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrAckIrq(dma, ret, XAXIDMA_DEVICE_TO_DMA);
	configASSERT(ret & XAXIDMA_IRQ_ALL_MASK);
    portYIELD_FROM_ISR(wake);
}


XGpio Gpio_reset_fifo;

void tskData(void *arg)
{
	//XTime t1, t2;
	//static u8 done=0;

	memset(temp0, 0, SMP_PER_FRAME * 8);
	memset(temp1, 0, SMP_PER_FRAME * 8);

	static XAxiDma DmaADC0, DmaADC1;
	int cntframe = 0;
	XStatus ret;
	XAxiDma_Config *Config;

	Config = XAxiDma_LookupConfig(XPAR_ADC0_AXI_DMA_ADC0_DEVICE_ID);
	configASSERT(Config);
	ret = XAxiDma_CfgInitialize(&DmaADC0, Config);
	configASSERT(ret == XST_SUCCESS);
	ret = XScuGic_Connect(&InterruptController, XPAR_FABRIC_ADC0_AXI_DMA_ADC0_S2MM_INTROUT_INTR,
					(Xil_InterruptHandler)FifoCh0Callback, &DmaADC0);
	configASSERT(ret == XST_SUCCESS);
	XAxiDma_IntrEnable(&DmaADC0, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

	Config = XAxiDma_LookupConfig(XPAR_ADC1_AXI_DMA_ADC0_DEVICE_ID);
	configASSERT(Config);
	ret = XAxiDma_CfgInitialize(&DmaADC1, Config);
	configASSERT(ret == XST_SUCCESS);
	ret = XScuGic_Connect(&InterruptController, XPAR_FABRIC_ADC1_AXI_DMA_ADC0_S2MM_INTROUT_INTR,
					(Xil_InterruptHandler)FifoCh1Callback, &DmaADC1);
	configASSERT(ret == XST_SUCCESS);
	XAxiDma_IntrEnable(&DmaADC1, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XScuGic_Enable(&InterruptController, XPAR_FABRIC_ADC0_AXI_DMA_ADC0_S2MM_INTROUT_INTR);
	XScuGic_Enable(&InterruptController, XPAR_FABRIC_ADC1_AXI_DMA_ADC0_S2MM_INTROUT_INTR);

	set_tlast_count(SMP_PER_FRAME);
	set_freq(30000);

	ret = XGpio_Initialize(&Gpio_reset_fifo, XPAR_AXI_GPIO_RESET_FIFO_DEVICE_ID);
	configASSERT(ret == XST_SUCCESS);
	XGpio_SetDataDirection(&Gpio_reset_fifo, 1, ~0x1);
	XGpio_SetDataDirection(&Gpio_reset_fifo, 2, ~0x3);
	XGpio_DiscreteClear(&Gpio_reset_fifo, 1, 1); 	//give 0 to reset fifo
	XGpio_DiscreteSet(&Gpio_reset_fifo, 1, 1);		//done reset
	XAxiDma_SimpleTransfer(&DmaADC0, (UINTPTR)temp0, SMP_PER_FRAME * 8 , XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_SimpleTransfer(&DmaADC1, (UINTPTR)temp1, SMP_PER_FRAME * 8 , XAXIDMA_DEVICE_TO_DMA);
	xSemaphoreTake(SemFifoCh0, portMAX_DELAY);
	xSemaphoreTake(SemFifoCh1, portMAX_DELAY);
	XAxiDma_SimpleTransfer(&DmaADC0, (UINTPTR)temp0, SMP_PER_FRAME * 8 , XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_SimpleTransfer(&DmaADC1, (UINTPTR)temp1, SMP_PER_FRAME * 8 , XAXIDMA_DEVICE_TO_DMA);//abandon first two frames
	xSemaphoreTake(SemFifoCh0, portMAX_DELAY);
	xSemaphoreTake(SemFifoCh1, portMAX_DELAY);
	Xil_DCacheInvalidateRange((UINTPTR)temp0, SMP_PER_FRAME * 8);
	Xil_DCacheInvalidateRange((UINTPTR)temp1, SMP_PER_FRAME * 8);

	while(1)
	{
		ret = XAxiDma_SimpleTransfer(&DmaADC1, (UINTPTR)temp0, SMP_PER_FRAME * 8 , XAXIDMA_DEVICE_TO_DMA);
		configASSERT(ret == XST_SUCCESS);
		ret = XAxiDma_SimpleTransfer(&DmaADC0, (UINTPTR)temp1, SMP_PER_FRAME * 8 , XAXIDMA_DEVICE_TO_DMA);
		configASSERT(ret == XST_SUCCESS);
		xSemaphoreTake(SemFifoCh0, portMAX_DELAY);
		xSemaphoreTake(SemFifoCh1, portMAX_DELAY);
		Xil_DCacheInvalidateRange((UINTPTR)temp0, SMP_PER_FRAME * 8);
		Xil_DCacheInvalidateRange((UINTPTR)temp1, SMP_PER_FRAME * 8);
		for (int i = 0 ; i < SMP_PER_FRAME; i++ )
		{
			QueueCh0.En(*((u48 *)(temp0+i)));
			QueueCh1.En(*((u48 *)(temp1+i)));
		}
		if (++cntframe == FRAME_PER_SECOND)
		{
			cntframe = 0;
			xSemaphoreGive(SemQueueCh0);

		}
	}
}

void InitData()
{
	XGpio_Initialize(&Gpio_reset_fifo, XPAR_AXI_GPIO_RESET_FIFO_DEVICE_ID);
	XGpio_DiscreteClear(&Gpio_reset_fifo, 2, 0x3);	//enlight both leds
	XGpio_DiscreteClear(&Gpio_reset_fifo, 1, 1);
	SemFifoCh0 = xSemaphoreCreateBinary();
	SemFifoCh1 = xSemaphoreCreateBinary();
	SemQueueCh0 = xSemaphoreCreateCounting(125 * 60 * 2, 0);
	//SemQueueCh1 = xSemaphoreCreateBinary();
	xTaskCreate(tskData, "TskData", tskDataStkSize, NULL, tskDataPrio, &tskDataHandle);
}



