/*
 * MenuDefines.h
 *
 *  Created on: Jun 19, 2020
 *      Author: loywong
 */

#ifndef SRC_TSKHIF_MENUDEFINES_H_
#define SRC_TSKHIF_MENUDEFINES_H_

#include "../EzMenu.h"

const unsigned int MIValLen = 20;

const unsigned int M1ICount = 2; // 7

//extern EzMenuItem<u8, MIValLen> miDeviceName;
extern EzMenuItem<u8, MIValLen> miDateTime;
extern EzMenuItem<u8, MIValLen> miChannelType;
extern EzMenuItem<u8, MIValLen> miChannelEnMark;
extern EzMenuItem<u8, MIValLen> miStatusBar;
extern EzMenuItem<u8, MIValLen> *m1Items[];
extern EzMenu<u8, MIValLen> menu1;

const unsigned int MDTEICount = 8;
extern EzMenuItem<u8, MIValLen> miYearEdit;
extern EzMenuItem<u8, MIValLen> miMonthEdit;
extern EzMenuItem<u8, MIValLen> miDayEdit;
extern EzMenuItem<u8, MIValLen> miHourEdit;
extern EzMenuItem<u8, MIValLen> miMinuteEdit;
extern EzMenuItem<u8, MIValLen> miSecondEdit;
extern EzMenuItem<u8, MIValLen> miDTEditOK;
extern EzMenuItem<u8, MIValLen> miDTEditCancel;
extern EzMenuItem<u8, MIValLen> *mDTEItems[];
extern EzMenu<u8, MIValLen> menuDTEdit;

extern EzMenu<u8, MIValLen> *CurrMenu;

void InitMenus();

#endif /* SRC_TSKHIF_MENUDEFINES_H_ */
