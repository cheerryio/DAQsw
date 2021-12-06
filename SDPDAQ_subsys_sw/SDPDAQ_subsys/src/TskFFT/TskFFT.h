#ifndef _TSKFFT_H_
#define _TSKFFT_H_

#include "../common.h"
#include "Complex.h"
#include "FFT.h"
#include "Queue.h"
#include "../TskSD/SD.h"
#include "../TskData/TskData.h"
#include <ff.h>

#define N 16384

extern TaskHandle_t tskFFTHandle;

void tskFFTInit();
#endif