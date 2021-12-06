/*
 * IicMaster.cpp
 *
 *  Created on: Jul 24, 2014
 *      Author: loywong
 */

// word:                                    1                                     |        0        |
// byte:    |          07          |          06          |    05     |    04     | 03..02 | 01..00 |
// bit:     | 31..26 |  25  |  24  | 23..18 |  17  |  16  |  15..08   |  07..00   | 31..16 | 15..00 |
// write:   |    X   | rxim | txim |    X   |   X  |   X  | rx irq th | tx irq th |    X   | tx cmd |
// read:    |    X   | rxim | txim |    X   | rxis | txis | rx fifo w | tx fifo w | rx dat |    X   |

#include "IicMaster.h"
#include <xil_types.h>
#include <xstatus.h>
#include <stddef.h>
#include <xil_io.h>
#include <xintc.h>
//#include <sys/intr.h>
//#include <sys/timer.h>
#include <stdio.h>
#include <stdarg.h>
#include <task.h>
//#include "../common.h"

void iic_master_isr(void *context)
{
    IicMaster *iic = (IicMaster *)context;
    unsigned char is = Xil_In8(iic->addr_intsts);
    if(0x01 & is)
    {
        if((iic->txCallback) != NULL)
        {
            iic->txCallback(iic->txcbCont);
        }
    }
    if(0x02 & is)
    {
        if((iic->rxCallback) != NULL)
        {
            iic->rxCallback(iic->rxcbCont);
        }
    }
}

#ifndef __XMK__
IicMaster::IicMaster(u32 base, XIntc * intc, u8 intId, void (*txeCallback)(void *), void *txeCbCont, void (*rxfCallback)(void *), void * rxfCbCont)
#else
IicMaster::IicMaster(u32 base, u8 intId, void (*txeCallback)(void *), void *txeCbCont, void (*rxfCallback)(void *), void * rxfCbCont)
#endif
{
	// TODO Auto-generated constructor stub
	int sts;

	this->busy = false;
	this->intc = intc;
    this->base = base;
    this->addr_wrcmd = base;
    this->addr_rddat = base + 2;
    this->addr_cmdlenth = base + 4;
    this->addr_datlenth = base + 6;
    this->addr_intsts = base + 8;
    this->addr_intmsk = base + 9;

    ///////////////////////////////////
//    Xil_Out8(addr_intmsk, 1);
//    sts = Xil_In8(addr_intmsk);
//    Xil_Out8(addr_intmsk, 2);
//    sts = Xil_In8(addr_intmsk);
//    Xil_Out8(addr_intmsk, 3);
//    sts = Xil_In8(addr_intmsk);
//    Xil_Out8(addr_intmsk, 7);
//    sts = Xil_In8(addr_intmsk);
    ///////////////////////////////////

    this->txCallback = txeCallback;
    this->txcbCont = txeCbCont;
    this->rxCallback = rxfCallback;
    this->rxcbCont = rxfCbCont;

    // set tx empty th = 0
    Xil_Out16(this->addr_cmdlenth, 0);
    // set rx full th = rx_fifo_len
    Xil_Out16(this->addr_datlenth, this->rx_fifo_len);

    while(this->DataLen() > 0)
    {
    	Xil_In16(this->addr_rddat);
    }

    if(NULL != txeCallback ||  NULL != rxfCallback)
    {
#ifndef __XMK__
    	sts = XIntc_Connect(intc, intId, iic_master_isr, (void *)this);
#else
    	sts = register_int_handler(intId, iic_master_isr, (void *)this);
#endif
    	if(sts != XST_SUCCESS)
    	{
    		printf("register isr error in IicMaster::IicMaster.\n");
    		delete this;
    		return;
    	}
#ifndef __XMK__
    	XIntc_Enable(intc, intId);
#else
    	enable_interrupt(intId);
#endif
    }
    if(NULL != txeCallback)
        this->EnableTxEmptyInt();
    if(NULL != rxfCallback)
        this->EnableRxFullInt();
}

IicMaster::~IicMaster() {
	// TODO Auto-generated destructor stub
}

void IicMaster::PushCmd(Cmd cmd, u8 data)
{
    // byte             means
    // 0_XXXXXXXX_1     write byte & read ack
    // 0_XXXXXXXX_0     write byte & write ack
    // 0_11111111_0     read byte & write ack
    // 0_11111111_1     read byte & read ack
    // 1_0XXXXXXX_X     start
    // 1_1XXXXXXX_X     stop
    u16 cmddata = ((u16)data << 1);
    int len;
    while((len = this->CmdLen()) >= this->tx_fifo_len)
    {
    }

    switch(cmd)
    {
    case Stop:
    	Xil_Out16(this->addr_wrcmd, 0x300);
    	this->busy = false;
        break;
    case Start:
    case Restart:
    	Xil_Out16(this->addr_wrcmd, 0x200);
    	this->busy = true;
        break;
    case WriteByte:
    	Xil_Out16(this->addr_wrcmd, cmddata | 0x1);
        break;
    case ReadByteAck:
    	Xil_Out16(this->addr_wrcmd, 0x1FE);
        break;
    case ReadByteNak:
    	Xil_Out16(this->addr_wrcmd, 0x1FF);
        break;
    case WriteByteAck:
    	Xil_Out16(this->addr_wrcmd, cmddata);
        break;
    default:
        break;
    }
}

void IicMaster::PopData(u8 *data, u8 *ack)
{
    u16 d;
    while(this->DataLen() == 0);
    d = Xil_In16(this->addr_rddat);
    *ack = !(d & 0x1);
    *data = d >> 1;
}

 u16 IicMaster::CmdLen()
{
    return Xil_In16(this->addr_cmdlenth);
}

 u16 IicMaster::DataLen()
{
    return Xil_In16(this->addr_datlenth);
}

 void IicMaster::EnableTxEmptyInt()
{
    u8 tmp = Xil_In8(this->addr_intmsk);
    tmp |= 0x01;
    Xil_Out8(this->addr_intmsk, tmp);
}

 void IicMaster::DisableTxEmptyInt()
{
    u8 tmp = Xil_In8(this->addr_intmsk);
    tmp &= 0xFE;
    Xil_Out8(this->addr_intmsk, tmp);
}

 void IicMaster::EnableRxFullInt()
{
    u8 tmp = Xil_In8(this->addr_intmsk);
    tmp |= 0x02;
    Xil_Out8(this->addr_intmsk, tmp);
}

 void IicMaster::DisableRxFullInt()
{
	u8 tmp = Xil_In8(this->addr_intmsk);
    tmp &= 0xFD;
    Xil_Out8(this->addr_intmsk, tmp);
}

 void IicMaster::SetTxEmptyIntThreshold(u16 th)
{
	Xil_Out16(this->addr_cmdlenth, th);
}

 void IicMaster::SetRxFullIntThreshold(u16 th)
{
	Xil_Out16(this->addr_datlenth, th);
}

void IicMaster::WriteRegPush(u8 saddr, u8 regStart, u8 regNum, ...)
{
	int i;
	u8 data;
	va_list vl;
	va_start(vl, regNum);
	this->PushCmd(IicMaster::Start);
	this->PushCmd(IicMaster::WriteByte, saddr);
	this->PushCmd(IicMaster::WriteByte, regStart);
	for(i = 0; i < regNum; i++)
	{
		data = (u8)va_arg(vl, int);
		this->PushCmd(IicMaster::WriteByte, data);
	}
	this->PushCmd(IicMaster::Stop);
	va_end(vl);
}

void IicMaster::WriteRegPush(u8 saddr, u8 regStart, u8 *data, u8 regNum)
{
	int i;
	this->PushCmd(IicMaster::Start);
	this->PushCmd(IicMaster::WriteByte, saddr);
	this->PushCmd(IicMaster::WriteByte, regStart);
	for(i = 0; i < regNum; i++)
	{
		this->PushCmd(IicMaster::WriteByte, data[i]);
	}
	this->PushCmd(IicMaster::Stop);
}

bool IicMaster::WriteRegPop(int regNum)
{
	u8 data;
	u8 ack;
	bool rtn = true;

	while(this->DataLen() < regNum + 2)
	{
		vTaskDelay(1);
	}

	for(regNum += 2; regNum > 0; regNum--)
	{
		this->PopData(&data, &ack);
		if(!ack)
			rtn = false;
	}
	return rtn;
}

void IicMaster::ReadRegPush(u8 saddr, u8 regStart, u8 regNum)
{
	int i;

	this->PushCmd(IicMaster::Start);
	this->PushCmd(IicMaster::WriteByte, saddr);
	this->PushCmd(IicMaster::WriteByte, regStart);

	this->PushCmd(IicMaster::Restart);
	this->PushCmd(IicMaster::WriteByte, saddr | 1);

	for(i = 0; i < regNum - 1; i++)
	{
		this->PushCmd(IicMaster::ReadByteAck);
	}
	this->PushCmd(IicMaster::ReadByteNak);
	this->PushCmd(IicMaster::Stop);
}

bool IicMaster::ReadRegPop(u8 regNum, ...)
{
	int i;
	u8 data, ack;
	bool rtn = true;
	va_list vl;

	while(this->DataLen() < regNum + 3)
	{
		vTaskDelay(1);
	}

	va_start(vl, regNum);

	this->PopData(&data, &ack);
	if(!ack) rtn = false;
	this->PopData(&data, &ack);
	if(!ack) rtn = false;
	this->PopData(&data, &ack);
	if(!ack) rtn = false;

	for(i = 0; i < regNum; i++)
	{
		this->PopData(&data, &ack);
		*va_arg(vl, u8 *) = data;
	}
	va_end(vl);
	return rtn;
}

bool IicMaster::ReadRegPop(u8 *data, u8 regNum)
{
	int i;
	u8 ack;
	bool rtn = true;

	while(this->DataLen() < regNum + 3)
	{
		vTaskDelay(1);
	}

	this->PopData(data, &ack);
	if(!ack) rtn = false;
	this->PopData(data, &ack);
	if(!ack) rtn = false;
	this->PopData(data, &ack);
	if(!ack) rtn = false;

	for(i = 0; i < regNum; i++)
	{
		this->PopData(data + i, &ack);
	}
	return rtn;
}
