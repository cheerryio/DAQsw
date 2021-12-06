/*
 * tskData.h
 *
 *  Created on: Mar 7, 2021
 *      Author: WuXJ
 */

#ifndef TSKDATA_H_
#define TSKDATA_H_

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "Queue.h"
#include "xgpio.h"

#pragma pack(1)
struct u48 {u16 a[3];};
#pragma pack()


extern XGpio Gpio_reset_fifo;

extern SemaphoreHandle_t SemFifoCh0, SemFifoCh1, SemQueueCh0, SemQueueCh1;
extern Queue<u48 , true> QueueCh0;
extern Queue<u48 , true> QueueCh1;

void tskData(void *arg);
void InitData();

#endif /* TSKDATA_H_ */
