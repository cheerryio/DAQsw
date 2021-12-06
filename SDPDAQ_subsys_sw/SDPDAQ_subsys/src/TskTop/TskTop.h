#ifndef _TSKTOP_H_
#define _TSKTOP_H_

#include <FreeRTOS.h>
#include <semphr.h>
#include "xil_types.h"
#include "../common.h"




void testGps();
void processTransSysMsg(topMode* mode, ctrlReg_t* param, tskTopMsg_t msg);

void tskTopInit();




#endif
