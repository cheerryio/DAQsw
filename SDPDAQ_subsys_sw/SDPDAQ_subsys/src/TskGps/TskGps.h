#ifndef _TSKGPS_H_
#define _TSKGPS_H_
#include "../common.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include "xil_types.h"
#include "Ublox.h"

char gpsGetChar();
time_t gettime_t(Ublox m8t);
void dbgDateTime(const tm* dateTime, u16 millis);
void dbgDateTime(const tm* dateTime);
void dbgPosition(float altitude, float latitude, float longtitude);


void tskGpsInit();


#endif
