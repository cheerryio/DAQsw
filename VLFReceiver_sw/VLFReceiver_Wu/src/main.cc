/*
 * main.cc
 *
 *  Created on: Mar 4, 2021
 *      Author: WuXJ
 */


/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "ff.h"
#include "SD.h"
#include "common.h"
#include "tskData.h"
#include "TskHif/TskHif.h"
#include "sleep.h"

XScuGic InterruptController;

void InitIntc()
{
	XScuGic_Config *GicConfig;
	GicConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
	XScuGic_CfgInitialize(&InterruptController, GicConfig,
					GicConfig->CpuBaseAddress);
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			&InterruptController);
	Xil_ExceptionEnable();

}

int main()
{
	//while(1);
	xil_printf("Hello\n\r");



	sleep_A9(1);
	InitIntc();
	InitData();
	InitSD();
	tskHifInit();
	//SDSpeedTest();
	//while(1);
	vTaskStartScheduler();

	while(1);

	return 0;
}
