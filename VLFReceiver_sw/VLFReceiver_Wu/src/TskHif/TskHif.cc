/*
 * TskHif.cc
 *
 *  Created on: Jun 18, 2020
 *      Author: loywong
 *
 *
 *  Modified on: Mar 9, 2021, WuXJ
 */

#include "TskHif.h"
//#include "../FifoIf/FifoIf.h"
#include "../common.h"
#include <xparameters.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "../IicMaster/IicMaster.h"
#include "../EzMenu.h"
#include "../oled12864/lq12864.h"
#include "PCF8563.h"
#include "../key.h"
#include "../IQ_Demodulator.h"
//#include "../TskDataLoop/TskDataLoop.h"
//#include <stdio.h>

//#include "../TskModem/TskDemod.h"
//#include "../TskModem/TskMod.h"
#include "MenuDefines.h"

const int tskHifPrio = 6;
const int tskHifStkSize = 16384;

TaskHandle_t tskHifHandle;

DateTime32 DateTime;


IicMaster *iic = NULL;
LQ12864 *oled = NULL;
PCF8563 *rtc = NULL;
LoyKey<25, 5> *keys[5];

extern XScuGic gic;

volatile u8 KeyStatus = 0;
char fileName[22];

void setPcf8574aLed(unsigned char msk, unsigned char led)
{
    static unsigned char val = 0x0;
    val = (val & ~msk) | led;
    iic->WriteRegPush(0b01110000, 1, 1, ~(val << 5));
    iic->WriteRegPop(1);
}

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
            if(act == EzMIAction::FinishedEdit)
            {
                if(item == &::miFreqEdit)
                {
                	//miFreqEdit.Disable();
                	set_freq(miFreqEdit.GetInt());
                }
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
                    menuDTEdit.SetCurrIdx(0);
                }
                CurrMenu = CurrMenu->CurrItem()->SubMenu();
            }
        }
    }
}

template<typename T = float>
T the_dB(T in)
{
    T db = (T)20 * std::log10(in * 14.142f);
    db = db > (T)-60 ? db : (T)60 + db * (T)2;
    return db;
}

void tskHif(void *arg)
{
    u8 rtn = 0;

    vTaskDelay(50);
    iic = new IicMaster(0x43C10000, &InterruptController, XPAR_FABRIC_AXI4LITE_BUFFERED_IIC_MASTER_0_VEC_ID, NULL, NULL, NULL, NULL);
    oled = new LQ12864(iic, 0b0111101);
    rtc = new PCF8563(iic, 0x51);
    // clear oled
    oled->BufFill(0);
    oled->FlushAll();
    iic->WaitIdle();
    // 0b0001_1111 --> reg3: 3 outputs for led & 5 inputs for key
    iic->WriteRegPush(0b01110000, 2, 1, 0x00);
    iic->WriteRegPop(1);
    iic->WriteRegPush(0b01110000, 3, 1, 0x1f);
    iic->WriteRegPop(1);
    iic->WaitIdle();
    vTaskDelay(50);
    // led0 on
    setPcf8574aLed(0b111, 0b001);
    vTaskDelay(250);
    setPcf8574aLed(0b111, 0b010);
    vTaskDelay(250);
    setPcf8574aLed(0b111, 0b100);
    vTaskDelay(250);
    setPcf8574aLed(0b111, 0b000);

    keys[0] = new LoyKey<25, 5>(getKey, (void *)0, keyAction, (void *)0x11);
    keys[1] = new LoyKey<25, 5>(getKey, (void *)1, keyAction, (void *)0x14);
    keys[2] = new LoyKey<25, 5>(getKey, (void *)2, keyAction, (void *)'\r');
    keys[3] = new LoyKey<25, 5>(getKey, (void *)3, keyAction, (void *)0x12);
    keys[4] = new LoyKey<25, 5>(getKey, (void *)4, keyAction, (void *)0x13);

    InitMenus();
    DateTime = PCF8563::DT32(20, 6, 19, 0, 0, 0);

    CurrMenu->Render(oled);
    oled->FlushAll();
    iic->WaitIdle();
    vTaskDelay(50);
    rtn = iic->DataLen();
    vTaskDelay(50);

    while(1)
    {
    	// read key value
        iic->ReadRegPush(0b01110000, 0, 1);
        iic->ReadRegPop(1, &rtn);
        KeyStatus = ~rtn;
        rtn = iic->DataLen();
        // tick keys
        for(int i = 0; i < 5; i++)
        {
            keys[i]->Tick();
        }

        // demod show
        //static int demodLedCnt = 0;
//        if(DemodSuccess)
//        {
//            DemodSuccess = false;
//        	sprintf(str, "%07lu", DemodData);
//            miDecDisp.SetValue(str);
//            swap(miDecDisp.BColor, miDecDisp.FColor);
//            setPcf8574aLed(0b010, 0b010);
//            demodLedCnt = 10;
//        }


        //////////////////////////
        //sprintf(str, "Start Rec.  %07lu", RMS);
        //miRecordBtn.SetValue(str);


        //if(demodLedCnt > 0)
        //{
        //    demodLedCnt--;
        //}
        //if(demodLedCnt == 1)
        //{
        //    swap(miDecDisp.BColor, miDecDisp.FColor);
         //   setPcf8574aLed(0b010, 0b000);
        //}

        // about mod
//        if(ModFinish)
//        {
//        	ModFinish = false;
//        	::miCodeEdit.Enable();
//            setPcf8574aLed(0b001, 0b000);
//        }
        static int cnt = 0;
        static u8 wday = 0;

        char str[MIValLen + 1];
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
//                DateTimeReady=true;
            }
            else
            {
                rtc->SetDateTime(DateTime, wday);
            }
        }

        CurrMenu->Render(oled);
        oled->FlushAll();

        vTaskDelay(19);
        iic->WaitIdle();
    }

    delete iic;
}

void tskHifInit()
{
    int ret;
    ret = xTaskCreate(tskHif, "TskHif", tskHifStkSize, NULL, tskHifPrio, &tskHifHandle);
    configASSERT(ret == pdPASS);
}
