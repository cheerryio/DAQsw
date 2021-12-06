/*
 * PCF8563.cc
 *
 *  Created on: Jun 19, 2020
 *      Author: loywong
 */

#include "../IicMaster/IicMaster.h"
#include "PCF8563.h"

DateTime32 PCF8563::DT32(u8 year, u8 month, u8 day, u8 hour, u8 min, u8 sec)
{
    DateTime32 rtn;
    rtn.Value = ((u32)year << 26) | ((u32)month << 22) | ((u32)day << 17) |
                ((u32)hour << 12) | ((u32)min << 6) | (u32)sec;
    return rtn;
}

PCF8563::PCF8563(IicMaster *iic, u8 saddr7b)
{
    this->iic = iic;
    this->saddr = saddr7b << 1;
}

bool PCF8563::GetDateTime(DateTime32 &dataTime, u8 &weekDay)
{
    bool lv;
    DateTime32 dt;
    u8 year, mon, day, wday, hr, min, sec;
    iic->ReadRegPush(saddr, 2, 7);
    iic->ReadRegPop(7, &sec, &min, &hr, &day, &wday, &mon, &year);
    dt.Second = (sec & 0xf) + 10 * (sec >> 4 & 0x7);
    dt.Minute = (min & 0xf) + 10 * (min >> 4 & 0x7);
    dt.Hour   = ( hr & 0xf) + 10 * ( hr >> 4 & 0x3);
    dt.Day    = (day & 0xf) + 10 * (day >> 4 & 0x3);
    weekDay   = wday & 0x7;
    dt.Month  = (mon & 0xf) + 10 * (mon >> 4 & 0x1);
    dt.YearFrom2000 = (year & 0xf) + 10 * (year >> 4) + 100 * (mon >> 7 & 0x1);
    lv = (sec & 0x80);
    if(!lv) dataTime = dt;
    return !lv;
}

void PCF8563::SetDateTime(DateTime32 dt, u8 weekDay)
{
    SetDateTime(dt.YearFrom2000 + 2000, dt.Month, dt.Day, weekDay, dt.Hour, dt.Minute, dt.Second);
}

void PCF8563::SetDateTime(u16 year, u8 month, u8 day, u8 wday, u8 hour, u8 min, u8 sec)
{
    sec   = sec   % 10 | sec   / 10 << 4;
    min   = min   % 10 | min   / 10 << 4;
    hour  = hour  % 10 | hour  / 10 << 4;
    day   = day   % 10 | day   / 10 << 4;
    month = month % 10 | month / 10 << 4 | (year >= 2100 ? 0x80 : 0x00);
    year  = year  % 100;
    year  = year  % 10 | year  / 10 << 4;
    iic->WriteRegPush(saddr, 2, 7, sec, min, hour, day, wday, month, year);
    iic->WriteRegPop(7);
}
