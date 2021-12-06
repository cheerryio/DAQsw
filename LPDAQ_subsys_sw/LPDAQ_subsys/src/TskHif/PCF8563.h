/*
 * PCF8563.h
 *
 *  Created on: Jun 19, 2020
 *      Author: loywong
 */

#ifndef SRC_TSKHIF_PCF8563_H_
#define SRC_TSKHIF_PCF8563_H_

#include <xil_types.h>
#include "../IicMaster/IicMaster.h"

union DateTime32
{
    u32 Value;
    struct
    {
        u32 Second          : 6;
        u32 Minute          : 6;
        u32 Hour            : 5;
        u32 Day             : 5;
        u32 Month           : 4;
        u32 YearFrom2000    : 6;
    };
};

class PCF8563
{
private:
    IicMaster *iic = NULL;
    u8 saddr = 0xa2;
public:
    PCF8563(IicMaster *iic, u8 saddr7b);
    bool GetDateTime(DateTime32 &dt, u8 &weekDay);
    void SetDateTime(DateTime32 dt, u8 weekDay);
    void SetDateTime(u16 year, u8 month, u8 day, u8 wday, u8 hour, u8 min, u8 sec);
    static DateTime32 DT32(u8 year, u8 month, u8 day, u8 hour, u8 min, u8 sec);
};


#endif /* SRC_TSKHIF_PCF8563_H_ */
