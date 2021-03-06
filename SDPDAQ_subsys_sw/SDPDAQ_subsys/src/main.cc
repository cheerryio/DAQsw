/*
 * Empty C++ Application
 */

#include <xil_printf.h>
#include <xuartps.h>
//#include "xuartlite.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <xscugic.h>

#include "common.h"
#include "TskData/TskData.h"
#include "TskTop/TskTop.h"
#include "TskTransUart/TskTransUart.h"
#include "TskSD/TskSD.h"
#include "TskGps/TskGps.h"
#include "TskFFT/TskFFT.h"

TaskHandle_t 		tskMainHandle;

void tskMain(void * arg)
{
	vTaskDelay(200);
	Xil_DCacheDisable();
	Xil_ICacheDisable();
	tskFFTInit();
	//tskTopInit();
	tskGpsInit();
    tskTransUartInit();
	//tskSDInit();
	tskDataInit();

	vTaskDelete(NULL);
}

int main()
{
    // init Periph
	initPeriph();

	xTaskCreate(tskMain, "TskMain", configMINIMAL_STACK_SIZE, NULL, 16, &tskMainHandle);

	vTaskStartScheduler();

	while(1);

	return 0;
}
