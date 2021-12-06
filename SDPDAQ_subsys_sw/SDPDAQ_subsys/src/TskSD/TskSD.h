#ifndef _TSKSD_H_
#define _TSKSD_H_
#include "../common.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include "xil_types.h"
#include "SD.h"

void tskSDInit();
void createRootDirWithPos(char* dirNamePtr, char* fileNamePtr, FIL *wf);
extern FIL wf;

#endif
