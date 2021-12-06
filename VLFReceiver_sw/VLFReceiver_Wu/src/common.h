/*
 * common.h
 *
 *  Created on: Jun 15, 2020
 *      Author: loywong
 *
 *  Modified on: Mar 9, 2021, WuXJ
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include "xscugic.h"

extern XScuGic InterruptController;

#define CHANNAL 2
#define BYTE_PER_SMP 3ll
#define SMP_PER_FRAME 4000ll
#define SMP_PER_SECOND 16000ll
#define FRAME_PER_SECOND 4ll// = SMP_PER_SECOND / SMP_PER_FRAME


template<typename T>
inline T sat(T x, T l, T h)
{
    return x > h ? h : x < l ? l : x;
}

template<typename T>
inline void swap(T x, T y)
{
	T t = x;
	x = y;
	y = t;
}

#endif /* SRC_COMMON_H_ */
