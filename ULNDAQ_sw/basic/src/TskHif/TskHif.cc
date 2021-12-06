/*
 * TskHif.cc
 *
 *  Created on: Jun 18, 2020
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
#include "../common.h"

const int tskHifPrio = 7;
const int tskHifStkSize = 32768;

TaskHandle_t tskHifHandle;

DateTime32 DateTime;

IicMaster *iic = NULL;
LQ12864 *oled = NULL;
PCF8563 *rtc = NULL;
LoyKey<25, 5> *keys[5];


extern XScuGic xInterruptController;
//extern XIntc Intc;
volatile u8 dcmLedSet = 0x60;// turn on b ,turn off g r .
volatile u8 KeyStatus = 0;
volatile u8 channelSta = 0xff;
volatile u8 tfCardSta = 0x03; // defualt set tf card ok

bool getKey(void *key)
{
    u32 n = (u32)key;
    return (KeyStatus & (1 << n));
}

void keyAction(void *key, bool curr, bool last, unsigned int holdTicks)
{
    if(curr)
    {
        EzMIAction act = CurrMenu->AnswerKey((char)(unsigned int)key);
        if(act != EzMIAction::Nothing)
        {
            EzMenuItem<u8, MIValLen> *item = CurrMenu->CurrItem();
//            menu.Render(oled);
//            oled->FlushAll();
//            iic->WaitIdle();
            if(act == EzMIAction::ValueChange)
            {
//                if(item == &miDacOutType)
//                {
//                    DacOut = (DACOutType)miDacOutType.CurrIdx();
//                }
//                else if(item == &miLoFreq)
//                {
//                    LoscFreq = miLoFreq.GetFloat();
//                }
//                else if(item == &::miBaseFc)
//                {
//                    BasebandFc = miBaseFc.GetFloat();
//                }
//                else if(item == &::miAgcTau)
//                {
//                    AgcTau = miAgcTau.GetFloat();
//                }
            }
            else if(act == EzMIAction::SubMenuEnter)
            {
                if(item == &miDTEditOK)
                {
                    rtc->SetDateTime(
                            (u16)miYearEdit.GetInt() + 2000,
                            (u8)miMonthEdit.GetInt(),
                            (u8)miDayEdit.GetInt(), 0,
                            (u8)miHourEdit.GetInt(),
                            (u8)miMinuteEdit.GetInt(),
                            (u8)miSecondEdit.GetInt());
                }
                else if(item == &miDateTime)
                {
                    char str[MIValLen + 1];
                    sprintf(str, "%02u", DateTime.YearFrom2000);
                    miYearEdit.SetValue(str);
                    sprintf(str, "%02u", DateTime.Month);
                    miMonthEdit.SetValue(str);
                    sprintf(str, "%02u", DateTime.Day);
                    miDayEdit.SetValue(str);
                    sprintf(str, "%02u", DateTime.Hour);
                    miHourEdit.SetValue(str);
                    sprintf(str, "%02u", DateTime.Minute);
                    miMinuteEdit.SetValue(str);
                    sprintf(str, "%02u", DateTime.Second);
                    miSecondEdit.SetValue(str);
                }
                CurrMenu = CurrMenu->CurrItem()->SubMenu();
            }
        }
    }
}

template<typename T>
void swap(T &a, T &b)
{
    T t;
    t = a;
    a = b;
    b = t;
}

void tskHif(void *arg)
{
    u8 rtn = 0;

    vTaskDelay(50);
//    iic = new IicMaster(0x43C10000, &gic, XPAR_FABRIC_AXI4LITE_BUFFERED_IIC_MASTER_0_VEC_ID, NULL, NULL, NULL, NULL);
    iic = new IicMaster(IIC_MASTER_BASEADDR, &xInterruptController, IIC_MASTER_IRQ, NULL, NULL, NULL, NULL);
    oled = new LQ12864(iic, 0b0111101);
    rtc = new PCF8563(iic, 0x51);
    oled->BufFill(0);
    oled->FlushAll();
    iic->WaitIdle();
    vTaskDelay(50);

    iic->WriteRegPush(0b01110000, 3, 1, 0x1f);
    iic->WaitIdle();

    keys[0] = new LoyKey<25, 5>(getKey, (void *)0, keyAction, (void *)0x11);
    keys[1] = new LoyKey<25, 5>(getKey, (void *)1, keyAction, (void *)0x14);
    keys[2] = new LoyKey<25, 5>(getKey, (void *)2, keyAction, (void *)'\r');
    keys[3] = new LoyKey<25, 5>(getKey, (void *)3, keyAction, (void *)0x12);
    keys[4] = new LoyKey<25, 5>(getKey, (void *)4, keyAction, (void *)0x13);

    InitMenus();
    DateTime = PCF8563::DT32(21, 10, 22, 0, 0, 0);

    CurrMenu->Render(oled);
    oled->FlushAll();
    iic->WaitIdle();
    vTaskDelay(50);
    rtn = iic->DataLen();
    vTaskDelay(50);

    while(1)
    {
        iic->ReadRegPush(0b01110000, 0, 1);
        iic->ReadRegPop(1, &rtn);
        KeyStatus = ~rtn;
        rtn = iic->DataLen();

        for(int i = 0; i < 5; i++)
        {
            keys[i]->Tick();
        }

        char str[MIValLen + 1];
        static int cnt = 0;
        static u8 wday = 0;

        if(cnt < 4) cnt++;
        else
        {
            cnt = 0;
            rtn = rtc->GetDateTime(DateTime, wday);
            if(rtn)
            {
                sprintf(str, "%4u-%02u-%02u %02u:%02u:%02u", DateTime.YearFrom2000 + 2000, DateTime.Month,
                        DateTime.Day, DateTime.Hour, DateTime.Minute, DateTime.Second);
                miDateTime.SetValue(str);
            }
            else
            {
                rtc->SetDateTime(DateTime, wday);
            }

            if(0x01 == tfCardSta)
            {
            	sprintf(str, "No TF Card");
            }
            else if( 0x02 == tfCardSta)
            {
            	sprintf(str, "TF Card OK");
            }
            else
            {
            	sprintf(str, "TF Card Searching");
            }
            miStatusBar.SetValue(str);
        }

        iic->WriteRegPush(0b01110000, 1, 1, dcmLedSet);
        CurrMenu->Render(oled);
        oled->FlushAll();

        vTaskDelay(20);
        iic->WaitIdle();
    }

    delete iic;
}

void tskHifInit()
{
    int ret;
//    SemDemodFifo = xSemaphoreCreateBinary();
    ret = xTaskCreate(tskHif, "TskHif", tskHifStkSize, NULL, tskHifPrio, &tskHifHandle);
    configASSERT(ret == pdPASS);
}
