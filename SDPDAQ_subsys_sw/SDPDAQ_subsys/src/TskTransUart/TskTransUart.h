#ifndef _TSKTRANSUART_H_
#define _TSKTRANSUART_H_

#include <FreeRTOS.h>
#include <semphr.h>
#include "../common.h"


void tskTransUartInit();
void cmdConfigStartTime(char *arg);
void cmdConfigEndTime(char *arg);
void cmdSetStart(char *arg);



#endif
