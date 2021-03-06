#include <FreeRTOS.h>
#include <task.h>
#include <xil_printf.h>
#include "common.h"
#include "TskHif/TskHif.h"
#include "TskData/TskData.h"
#include "TskSD/TskSD.h"

TaskHandle_t tskMainHandle;

void tskMain(void * arg)
{
	vTaskDelay(200);
	tskSDInit();
    tskHifInit();
    tskDataInit();
	vTaskDelete(NULL);
}

int main()
{
    initPeriph();

	xTaskCreate(tskMain, "TskMain", configMINIMAL_STACK_SIZE, NULL, 16, &tskMainHandle);
	vTaskStartScheduler();
	while(1);

	return 0;
}
