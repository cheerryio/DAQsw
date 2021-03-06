/*
 * MenuDefines.cc
 *
 *  Created on: Jun 19, 2020
 *      Author: loywong
 */


#include "TskHif.h"
#include <xparameters.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "../IicMaster/IicMaster.h"
#include "../EzMenu.h"
#include "../oled12864/lq12864.h"
#include "PCF8563.h"
#include "../key.h"
#include <cmath>
#include <stdio.h>
#include "MenuDefines.h"

const u8 clrWhite = 1, clrBlack = 0;

// ======== menu 1 ========
//const char *miDeviceNameStr = (char *)"      DAQ1245123";
//EzMenuItem<u8, MIValLen> miDeviceName(miDeviceNameStr);

const char *miChannelTypeStr = (char *)"IEPE      DIFF";
EzMenuItem<u8, MIValLen> miChannelType(miChannelTypeStr);
//
//const char *miChannelEnMarkStr = (char *)" 1 2 3 4   1 2 3 4";
const char *miChannelEnMarkStr = (char *)"";
EzMenuItem<u8, MIValLen> miChannelEnMark(miChannelEnMarkStr);

const char *miStatusBarStr = (char *)"";
EzMenuItem<u8, MIValLen> miStatusBar(miStatusBarStr);

const char *miDateTimeStr = (char *)"2021-08-04 17:00:00";
extern EzMenu<u8, MIValLen> menuDTEdit;
EzMenuItem<u8, MIValLen> miDateTime(miDateTimeStr, &menuDTEdit);

//EzMenuItem<u8, MIValLen> *m1Items[M1ICount] = {&miDeviceName, &miDateTime, &miChannelType, &miChannelEnMark };
//EzMenuItem<u8, MIValLen> *m1Items[M1ICount] = {&miDeviceInfo, &miChannelInfo, &miDateTime };
//EzMenuItem<u8, MIValLen> *m1Items[M1ICount] = {&miDateTime, &miChannelType, &miChannelEnMark };
EzMenuItem<u8, MIValLen> *m1Items[M1ICount] = {&miDateTime, &miStatusBar };
EzMenu<u8, MIValLen> menu1((char *)"****  ULNDAQ  ****", m1Items, M1ICount);

// ======== menu data time edit ========
const char *miYearEditStr = (char *)"00";
EzMenuItem<u8, MIValLen> miYearEdit(miYearEditStr, 00, 63, 2);

const char *miMonthEditStr = (char *)"01";
EzMenuItem<u8, MIValLen> miMonthEdit(miMonthEditStr, 01, 12, 2);

const char *miDayEditStr = (char *)"01";
EzMenuItem<u8, MIValLen> miDayEdit(miDayEditStr, 01, 31, 2);

const char *miHourEditStr = (char *)"00";
EzMenuItem<u8, MIValLen> miHourEdit(miHourEditStr, 00, 23, 2);

const char *miMinuteEditStr = (char *)"00";
EzMenuItem<u8, MIValLen> miMinuteEdit(miMinuteEditStr, 00, 59, 2);

const char *miSecondEditStr = (char *)"00";
EzMenuItem<u8, MIValLen> miSecondEdit(miSecondEditStr, 00, 59, 2);

const char *miDTEditOKStr = (char *)"OK";
EzMenuItem<u8, MIValLen> miDTEditOK(miDTEditOKStr, &menu1);

const char *miDTEditCancelStr = (char *)"Cancel";
EzMenuItem<u8, MIValLen> miDTEditCancel(miDTEditCancelStr, &menu1);

EzMenuItem<u8, MIValLen> *mDTEItems[MDTEICount] = {
        &miYearEdit, &miMonthEdit, &miDayEdit, &miHourEdit,
        &miMinuteEdit, &miSecondEdit, &miDTEditOK, &miDTEditCancel};
EzMenu<u8, MIValLen> menuDTEdit((char *)"* Adj. Data & Time *", mDTEItems, MDTEICount);

// curr menu
EzMenu<u8, MIValLen> *CurrMenu = &menu1;

void initMenu1()
{
//	miDeviceName.BColor = clrBlack;
//	miDeviceName.FColor = clrWhite;
//	miDeviceName.DisBColor = clrBlack;
//	miDeviceName.DisFColor = clrWhite;
//	miDeviceName.FocBColor = clrWhite;
//	miDeviceName.FocFColor = clrBlack;
//	miDeviceName.PosX = 7;
//	miDeviceName.PosY = 3;
//	miDeviceName.Width = 128;
//	miDeviceName.Height = 10;
////    miDeviceName.Prefix = (char *)"      DAQ1245123";
//	miDeviceName.CanFocus = false;

    miDateTime.BColor = clrBlack;
    miDateTime.FColor = clrWhite;
    miDateTime.DisBColor = clrBlack;
    miDateTime.DisFColor = clrWhite;
    miDateTime.FocBColor = clrWhite;
    miDateTime.FocFColor = clrBlack;
    miDateTime.PosX = 7;
    miDateTime.PosY = 16;
    miDateTime.Width = 128;
    miDateTime.Height = 10;

    miChannelType.BColor = clrBlack;
    miChannelType.FColor = clrWhite;
    miChannelType.DisBColor = clrBlack;
    miChannelType.DisFColor = clrWhite;
    miChannelType.FocBColor = clrWhite;
    miChannelType.FocFColor = clrBlack;
    miChannelType.PosX = 7+15;
    miChannelType.PosY = 32;
    miChannelType.Width = 128;
    miChannelType.Height = 10;
//    miChannelType.Prefix = (char *)"IEPE      DIFF";
    miChannelType.CanFocus = false;

    miChannelEnMark.BColor = clrBlack;
    miChannelEnMark.FColor = clrWhite;
    miChannelEnMark.DisBColor = clrBlack;
    miChannelEnMark.DisFColor = clrWhite;
    miChannelEnMark.FocBColor = clrWhite;
    miChannelEnMark.FocFColor = clrBlack;
    miChannelEnMark.PosX = 7;
    miChannelEnMark.PosY = 48;
    miChannelEnMark.Width = 128;
    miChannelEnMark.Height = 10;
//    miChannelEnMark.Prefix = (char *)" 1 2 3 4   1 2 3 4";
    miChannelEnMark.CanFocus = false;

    miStatusBar.BColor = clrBlack;
    miStatusBar.FColor = clrWhite;
    miStatusBar.DisBColor = clrBlack;
    miStatusBar.DisFColor = clrWhite;
    miStatusBar.FocBColor = clrWhite;
    miStatusBar.FocFColor = clrBlack;
    miStatusBar.PosX = 7;
    miStatusBar.PosY = 48;
    miStatusBar.Width = 128;
    miStatusBar.Height = 10;
//    miChannelEnMark.Prefix = (char *)" 1 2 3 4   1 2 3 4";
    miChannelEnMark.CanFocus = false;

    menu1.FocusFillColor = clrBlack;
    menu1.FocusLineColor = clrWhite;
    menu1.UnfocFillColor = clrBlack;
    menu1.UnfocLineColor = clrWhite;
    menu1.PosX = 0;
    menu1.PosY = 0;
    menu1.Height = 64;
    menu1.Width = 127;
    menu1.IsFocused = true;
}

void initMenuDTEdit()
{
    miYearEdit.BColor = clrBlack;
    miYearEdit.FColor = clrWhite;
    miYearEdit.DisBColor = clrBlack;
    miYearEdit.DisFColor = clrWhite;
    miYearEdit.FocBColor = clrWhite;
    miYearEdit.FocFColor = clrBlack;
    miYearEdit.PosX = 7;
    miYearEdit.PosY = 9;
    miYearEdit.Width = 64;
    miYearEdit.Height = 11;
    miYearEdit.Prefix = (char *)"Year: 20";

    miMonthEdit.BColor = clrBlack;
    miMonthEdit.FColor = clrWhite;
    miMonthEdit.DisBColor = clrBlack;
    miMonthEdit.DisFColor = clrWhite;
    miMonthEdit.FocBColor = clrWhite;
    miMonthEdit.FocFColor = clrBlack;
    miMonthEdit.PosX = 7;
    miMonthEdit.PosY = 18;
    miMonthEdit.Width = 64;
    miMonthEdit.Height = 11;
    miMonthEdit.Prefix = (char *)"Month:  ";

    miDayEdit.BColor = clrBlack;
    miDayEdit.FColor = clrWhite;
    miDayEdit.DisBColor = clrBlack;
    miDayEdit.DisFColor = clrWhite;
    miDayEdit.FocBColor = clrWhite;
    miDayEdit.FocFColor = clrBlack;
    miDayEdit.PosX = 7;
    miDayEdit.PosY = 27;
    miDayEdit.Width = 64;
    miDayEdit.Height = 11;
    miDayEdit.Prefix = (char *)"Day:    ";

    miHourEdit.BColor = clrBlack;
    miHourEdit.FColor = clrWhite;
    miHourEdit.DisBColor = clrBlack;
    miHourEdit.DisFColor = clrWhite;
    miHourEdit.FocBColor = clrWhite;
    miHourEdit.FocFColor = clrBlack;
    miHourEdit.PosX = 7;
    miHourEdit.PosY = 36;
    miHourEdit.Width = 64;
    miHourEdit.Height = 11;
    miHourEdit.Prefix = (char *)"Hour:   ";

    miMinuteEdit.BColor = clrBlack;
    miMinuteEdit.FColor = clrWhite;
    miMinuteEdit.DisBColor = clrBlack;
    miMinuteEdit.DisFColor = clrWhite;
    miMinuteEdit.FocBColor = clrWhite;
    miMinuteEdit.FocFColor = clrBlack;
    miMinuteEdit.PosX = 7;
    miMinuteEdit.PosY = 45;
    miMinuteEdit.Width = 64;
    miMinuteEdit.Height = 11;
    miMinuteEdit.Prefix = (char *)"Minute: ";

    miSecondEdit.BColor = clrBlack;
    miSecondEdit.FColor = clrWhite;
    miSecondEdit.DisBColor = clrBlack;
    miSecondEdit.DisFColor = clrWhite;
    miSecondEdit.FocBColor = clrWhite;
    miSecondEdit.FocFColor = clrBlack;
    miSecondEdit.PosX = 7;
    miSecondEdit.PosY = 54;
    miSecondEdit.Width = 64;
    miSecondEdit.Height = 11;
    miSecondEdit.Prefix = (char *)"Second: ";

    miDTEditOK.BColor = clrBlack;
    miDTEditOK.FColor = clrWhite;
    miDTEditOK.DisBColor = clrBlack;
    miDTEditOK.DisFColor = clrWhite;
    miDTEditOK.FocBColor = clrWhite;
    miDTEditOK.FocFColor = clrBlack;
    miDTEditOK.PosX = 88;
    miDTEditOK.PosY = 45;
    miDTEditOK.Width = 64;
    miDTEditOK.Height = 11;

    miDTEditCancel.BColor = clrBlack;
    miDTEditCancel.FColor = clrWhite;
    miDTEditCancel.DisBColor = clrBlack;
    miDTEditCancel.DisFColor = clrWhite;
    miDTEditCancel.FocBColor = clrWhite;
    miDTEditCancel.FocFColor = clrBlack;
    miDTEditCancel.PosX = 88;
    miDTEditCancel.PosY = 54;
    miDTEditCancel.Width = 64;
    miDTEditCancel.Height = 11;

    menuDTEdit.FocusFillColor = clrBlack;
    menuDTEdit.FocusLineColor = clrWhite;
    menuDTEdit.UnfocFillColor = clrBlack;
    menuDTEdit.UnfocLineColor = clrWhite;
    menuDTEdit.PosX = 0;
    menuDTEdit.PosY = 0;
    menuDTEdit.Height = 64;
    menuDTEdit.Width = 127;
    menuDTEdit.IsFocused = true;
}

void InitMenus()
{
    initMenu1();
    initMenuDTEdit();
}
