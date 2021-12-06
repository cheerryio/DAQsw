/*
 * IicMaster.h
 *
 *  Created on: Jul 24, 2014
 *      Author: loywong
 */

#ifndef IICMASTER_H_
#define IICMASTER_H_

#include <xil_io.h>
#include <xil_types.h>
#include <xscugic.h>
//#include <xintc.h>

class IicMaster {
public:
	enum Cmd
	{
	    Stop = 0,
	    Start,
	    Restart,
	    WriteByte,
	    ReadByteAck,
	    WriteByteAck,
	    ReadByteNak
	};
#ifdef PLATFORM_ZYNQ
	IicMaster(u32 base, XScuGic *intc, u8 intId, void (*txeCallback)(void *), void *txeCbCont, void (*rxfCallback)(void *), void * rxfCbCont);
#elif __XMK__
	IicMaster(u32 base, u8 intId, void (*txeCallback)(void *), void *txeCbCont, void (*rxfCallback)(void *), void * rxfCbCont);
#else
	IicMaster(u32 base, XIntc *intc, u8 intId, void (*txeCallback)(void *), void *txeCbCont, void (*rxfCallback)(void *), void * rxfCbCont);
#endif
	virtual ~IicMaster();
	void PushCmd(Cmd c, u8 data = 0);
	void PopData(u8 *data, u8 *ack);
	static const u16 tx_fifo_len = 1024;
	static const u16 rx_fifo_len = 2048;
	u16 CmdLen();
	u16 DataLen();

	void EnableTxEmptyInt();
	void DisableTxEmptyInt();
	void EnableRxFullInt();
	void DisableRxFullInt();

    void SetTxEmptyIntThreshold(u16 th);
    void SetRxFullIntThreshold(u16 th);

    void WriteRegPush(u8 saddr, u8 regStart, u8 regNum, ...);
    void WriteRegPush(u8 saddr, u8 regStart, u8 *data, u8 regNum);
    bool WriteRegPop(int regNum);

    void ReadRegPush(u8 saddr, u8 regStart, u8 regNum);
    bool ReadRegPop(u8 regNum, ...);
    bool ReadRegPop(u8 *data, u8 regNum);

    void WaitIdle();

private:
	u32 base;
#ifdef PLATFORM_ZYNQ
	XScuGic *intc;
#else
	XIntc *intc;
#endif
	u32 addr_wrcmd;
	u32 addr_rddat;
	u32 addr_cmdlenth;
	u32 addr_datlenth;
	u32 addr_intsts;
	u32 addr_intmsk;
	void (*txCallback)(void *);
	void *txcbCont;
	void (*rxCallback)(void *);
	void *rxcbCont;

	bool busy;

	friend void iic_master_isr(void *context);

};

#endif /* IICMASTER_H_ */
