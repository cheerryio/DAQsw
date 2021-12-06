#ifndef __TSKDATA_H__
#define __TSKDATA_H__

#include <FreeRTOS.h>
#include <semphr.h>
#include <xparameters.h>
#include <stdio.h>

#define DATABUF_NUM 60
#define DMA_TRANS_SIZE 10240                ///< dma一次搬运数据数量

extern SemaphoreHandle_t semDataBufWr;
extern SemaphoreHandle_t semDataBufRd;
extern u8 databuf[DATABUF_NUM][DMA_TRANS_SIZE*3] __attribute__((aligned(128)));

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

void tskDataInit();

#endif