#include <FreeRTOS.h>
#include <task.h>
#include <xil_printf.h>
#include "common.h"
#include "TskData/TskData.h"
#include "TskSD/TskSD.h"
#include "TskHif/TskHif.h"

TaskHandle_t tskMainHandle;

void tskMain(void *arg)
{
	vTaskDelay(200);
    tskDataInit();
    tskSDInit();
	tskHifInit();
	vTaskDelete(NULL);
}

int main()
{
    initPeriph();

	xTaskCreate(tskMain, "TskMain", 16384, NULL, 16, &tskMainHandle);
	vTaskStartScheduler();
	while(1);

	return 0;
}
