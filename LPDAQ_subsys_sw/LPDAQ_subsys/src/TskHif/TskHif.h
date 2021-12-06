/*
 * TskHif.h
 *
 *  Created on: Jun 18, 2020
 *      Author: loywong
 */

#ifndef SRC_TSKHIF_TSKHIF_H_
#define SRC_TSKHIF_TSKHIF_H_

#include <xil_types.h>
#include "PCF8563.h"

// for fatfs create file
extern DateTime32 DateTime;

// dcm led set value.
// bit set 0 turn on, set 1 turn off .
// D7~D0 -> bgrx_xxxx , x do not care.
extern volatile u8 dcmLedSet;

// D7 ~ D0 -> diff ch4/3/2/1 iepe ch4/3/2/1
extern volatile u8 channelSta;

// 0x01 TF Card error , 0x02 TF Card ok.
extern volatile u8 tfCardSta;

void tskHifInit();

//void tskHif(void *arg);

#endif /* SRC_TSKHIF_TSKHIF_H_ */
