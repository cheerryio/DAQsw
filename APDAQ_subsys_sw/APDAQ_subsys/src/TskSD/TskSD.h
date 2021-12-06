#ifndef __TSKSD_H__
#define __TSKSD_H__
#include "../common.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include "xil_types.h"
#include "SD.h"
#include "../TskHif/TskHif.h"

extern SemaphoreHandle_t semSDReady;
void tskSDInit();


#endif
