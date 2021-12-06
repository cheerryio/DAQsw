#include "xparameters.h"
#include "XilPeriph/XilGpio.h"
#include "XilPeriph/XilPsDma.h"
#include "TskSD/SD.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "xscugic.h"

#define DEBUG 0

void initPeriph();
