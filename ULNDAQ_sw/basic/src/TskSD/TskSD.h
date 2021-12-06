#ifndef __TSKSD_H__
#define __TSKSD_H__
#include "../common.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include "xil_types.h"
#include "SD.h"
#include "../TskHif/TskHif.h"

extern SemaphoreHandle_t semSDReady;
extern unsigned int databuf_rd_p[CHANNEL_NUM];
void tskSDInit();

#define DMC_ALL_LED_OFF \
    dcmLedSet |= 0xe0; // 0b111_00000  dcm all led turn off

#define DCM_RED_LED_ON \
    dcmLedSet &= 0xc0; // 0b110_00000 dcm red led turn on

#define DCM_RED_LED_OFF \
    dcmLedSet |= 0x20; // 0b001_00000 dcm red led turn off

#define DCM_GREEN_LED_ON \
    dcmLedSet &= 0xa0;    // 0b101_00000 dcm green led turn on

#define DCM_GREEN_LED_OFF \
    dcmLedSet |=0x40;

#define DCM_GREEN_TOGGLE \
    dcmLedSet=dcmLedSet&0x40?dcmLedSet&0xa0:dcmLedSet|0x40;   // 0b100_00000 0b011_00000 toggle dcm green led

#define DCM_YELLOW_LED_ON \
    dcmLedSet&=0x60;  // 0b011_00000 set dcm yellow led on
#define DCM_YELLOW_LED_OFF \
    dcmLedSet|=0x80;    // 0b100_00000 set dcm yellow led off

typedef struct TskSDCmd{
    unsigned int channnelNum;

} tskSDCmd_t;

#endif
