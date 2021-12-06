/******************************************************************************
*
* Copyright (C) 2013 - 2016 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file diskio.c
*		This file is the glue layer between file system and
*		driver.
*		Description related to SD driver:
*		Process to use file system with SD
*		Select xilffs in SDK when creating a BSP
*		In SDK, set "fs_interface" to 1 to select SD interface.
*		This glue layer can currently be used only with one
*		SD controller enabled.
*		In order to use eMMC, in SDK set "Enable MMC" to 1. If not,
*		SD support is enabled by default.
*
*		Description:
*		This glue layer initializes the host controller and SD card
*		in disk_initialize. If SD card supports it, 4-bit mode and
*		high speed mode will be enabled.
*		The default block size is 512 bytes.
*		disk_read and disk_write functions are used to read and
*		write files using ADMA2 in polled mode.
*		The file system can be used to read from and write to an
*		SD card that is already formatted as FATFS.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a hk   10/17/13 First release
* 2.0   hk   02/12/14 Corrected status check in disk initialize. CR# 772072.
* 2.1   hk   04/16/14 Move check for ExtCSD high speed bit set inside if
*                     condition for high speed support.
*                     Include xil_types.h irrespective of xsdps.h. CR# 797086.
* 2.2   hk   07/28/14 Make changes to enable use of data cache.
* 3.0	sk	 12/04/14 Added support for micro SD without
* 					  WP/CD. CR# 810655.
*					  Make changes for prototypes of disk_read and
*					  disk_write according to latest version.
*			 12/15/14 Modified the code according to MISRAC 2012 Compliant.
*					  Updated the FatFs to R0.10b
*					  Removed alignment for local buffers as CacheInvalidate
*					  will take care of it.
*		sg   03/03/15 Added card detection check logic
*		     04/28/15 Card detection only in case of card detection signal
* 3.1   sk   06/04/15 Added support for SD1.
* 3.2   sk   11/24/15 Considered the slot type before checking the CD/WP pins.
* 3.3   sk   04/01/15 Added one second delay for checking CD pin.
* 3.4   sk   06/09/16 Added support for mkfs.
* 3.8   mj   07/31/17 Added support for RAM based FATfs.
*       mn   12/04/17 Resolve errors in XilFFS for ARMCC compiler
*
* </pre>
*
* @note
*
******************************************************************************/
#include "diskio.h"
#include "ff.h"
#include "xil_types.h"
#include "FreeRTOS.h"

#ifdef FILE_SYSTEM_INTERFACE_SD
#include "../sdc_ctrl/mmc.h"		/* SD device driver */
struct mmc * ocsdc_mmc_init(int base_addr, int clk_freq);
#endif
#include "sleep.h"
#include "xil_printf.h"
#include "mb_interface.h"

#include "../TskHif/TskHif.h"
//#include "../TskHif/PCF8563.h"

#define HIGH_SPEED_SUPPORT	0x01U
#define WIDTH_4_BIT_SUPPORT	0x4U
#define SD_CLK_25_MHZ		25000000U
#define SD_CLK_26_MHZ		26000000U
#define SD_CLK_52_MHZ		52000000U
#define EXT_CSD_DEVICE_TYPE_BYTE	196
#define EXT_CSD_4_BIT_WIDTH_BYTE	183
#define EXT_CSD_HIGH_SPEED_BYTE		185
#define EXT_CSD_DEVICE_TYPE_HIGH_SPEED	0x3
#define SD_CD_DELAY		10000U

#ifdef FILE_SYSTEM_INTERFACE_RAM
#include "xparameters.h"

static char *dataramfs = NULL;

#define BLOCKSIZE       1U
#define SECTORSIZE      512U
#define SECTORCNT       (RAMFS_SIZE / SECTORSIZE)
#endif

/*--------------------------------------------------------------------------

	Public Functions

---------------------------------------------------------------------------*/

/*
 * Global variables
 */
static DSTATUS Stat[2] = {STA_NOINIT, STA_NOINIT};	/* Disk status */

#ifdef FILE_SYSTEM_INTERFACE_SD
static struct mmc* sddrv;
//static XSdPs SdInstance[2];
//static u32 BaseAddress;
//static u32 CardDetect;
//static u32 WriteProtect;
//static u32 SlotType[2];
//static u8 HostCntrlrVer[2];
#endif

#ifdef __ICCARM__
#pragma data_alignment = 32
static u8 ExtCsd[512];
#pragma data_alignment = 4
#else
static u8 ExtCsd[512] __attribute__ ((aligned(32)));
#endif

/*-----------------------------------------------------------------------*/
/* Get Disk Status							*/
/*-----------------------------------------------------------------------*/

/*****************************************************************************/
/**
*
* Gets the status of the disk.
* In case of SD, it checks whether card is present or not.
*
* @param	pdrv - Drive number
*
* @return
*		0		Status ok
*		STA_NOINIT	Drive not initialized
*		STA_NODISK	No medium in the drive
*		STA_PROTECT	Write protected
*
* @note		In case Card detect signal is not connected,
*		this function will not be able to check if card is present.
*
******************************************************************************/
DSTATUS disk_status (
		BYTE pdrv	/* Drive number (0) */
)
{
	DSTATUS s = Stat[pdrv];

	configASSERT(pdrv == 0);

	if(sddrv->has_init)
		s = 0;
	else
		s = STA_NOINIT;

    Stat[pdrv] = s;
    return s;
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive						 */
/*-----------------------------------------------------------------------*/
/*****************************************************************************/
/**
*
* Initializes the drive.
* In case of SD, it initializes the host controller and the card.
* This function also selects additional settings such as bus width,
* speed and block size.
*
* @param	pdrv - Drive number
*
* @return	s - which contains an OR of the following information
*		STA_NODISK	Disk is not present
*		STA_NOINIT	Drive not initialized
*		STA_PROTECT	Drive is write protected
*		0 or only STA_PROTECT both indicate successful initialization.
*
* @note
*
******************************************************************************/
DSTATUS disk_initialize (
		BYTE pdrv	/* Physical drive number (0) */
)
{
	configASSERT(pdrv == 0);	// support only one disk now

	//init ocsdc driver
	sddrv = ocsdc_mmc_init(XPAR_PERIPHERALS_WB_SDC_CTRL_BASEADDR, XPAR_CPU_M_AXI_DP_FREQ_HZ);
    configASSERT(sddrv);
//	if (!sddrv) {
//		printf("ocsdc_mmc_init failed\n\r");
//		return -1;
//	}
//	printf("ocsdc_mmc_init success\n\r");

	sddrv->has_init = 0;
	int err = mmc_init(sddrv);
//	configASSERT(!err);
//	configASSERT(sddrv->has_init);
	if(err != 0 || sddrv->has_init == 0 )
	{
//		tfCardSta = 0x01; // set tfcard status error.
	}
//	if (err != 0 || sddrv->has_init == 0) {
//		printf("mmc_init failed\n\r");
//		return -1;
//	}
//	printf("mmc_init success\n\r");

	print_mmcinfo(sddrv);
	Stat[pdrv] = 0;
	return 0;

}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)							 */
/*-----------------------------------------------------------------------*/
/*****************************************************************************/
/**
*
* Reads the drive
* In case of SD, it reads the SD card using ADMA2 in polled mode.
*
* @param	pdrv - Drive number
* @param	*buff - Pointer to the data buffer to store read data
* @param	sector - Start sector number
* @param	count - Sector count
*
* @return
*		RES_OK		Read successful
*		STA_NOINIT	Drive not initialized
*		RES_ERROR	Read not successful
*
* @note
*
******************************************************************************/
int rdcnt = 0;
DRESULT disk_read (
		BYTE pdrv,	/* Physical drive number (0) */
		BYTE *buff,	/* Pointer to the data buffer to store read data */
		DWORD sector,	/* Start sector number (LBA) */
		UINT count	/* Sector count (1..128) */
)
{
	rdcnt++;
	DSTATUS s;
#ifdef FILE_SYSTEM_INTERFACE_SD
//	s32 Status;
	DWORD LocSector = sector;
#endif

	configASSERT(pdrv == 0);
	s = disk_status(pdrv);

	if ((s & STA_NOINIT) != 0U) {
		return RES_NOTRDY;
	}
	if (count == 0U) {
		return RES_PARERR;
	}

#ifdef FILE_SYSTEM_INTERFACE_SD
	if(0 == mmc_bread(sddrv, LocSector, count, buff))
	{
		return RES_ERROR;
	}


//	/* Convert LBA to byte address if needed */
//	if ((SdInstance[pdrv].HCS) == 0U) {
//		LocSector *= (DWORD)XSDPS_BLK_SIZE_512_MASK;
//	}
//
//	Status  = XSdPs_ReadPolled(&SdInstance[pdrv], (u32)LocSector, count, buff);
//	if (Status != XST_SUCCESS) {
//		return RES_ERROR;
//	}
#endif

#ifdef FILE_SYSTEM_INTERFACE_RAM
	memcpy(buff, dataramfs + (sector * SECTORSIZE), count * SECTORSIZE);
#endif

    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions						*/
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,				/* Physical drive number (0) */
	BYTE cmd,				/* Control code */
	void *buff				/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_OK;

#ifdef FILE_SYSTEM_INTERFACE_SD
	void *LocBuff = buff;
	configASSERT(pdrv == 0);
	if ((disk_status(pdrv) & STA_NOINIT) != 0U) {	/* Check if card is in the socket */
		return RES_NOTRDY;
	}

	res = RES_ERROR;
	switch (cmd) {
		case (BYTE)CTRL_SYNC :	/* Make sure that no pending write process */
			res = RES_OK;
			break;

		case (BYTE)GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
//			(*((DWORD *)(void *)LocBuff)) = (DWORD)SdInstance[pdrv].SectorCount;
			(*((DWORD *)(void *)LocBuff)) = (DWORD)(sddrv->capacity / sddrv->read_bl_len);
			res = RES_OK;
			break;

		case (BYTE)GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
//			(*((DWORD *)((void *)LocBuff))) = ((DWORD)128);
			(*((DWORD *)((void *)LocBuff))) = ((DWORD)1);
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
			break;
	}
#endif

#ifdef FILE_SYSTEM_INTERFACE_RAM
	switch (cmd) {
	case (BYTE)CTRL_SYNC:
		break;
	case (BYTE)GET_BLOCK_SIZE:
		*(WORD *)buff = BLOCKSIZE;
		break;
	case (BYTE)GET_SECTOR_SIZE:
		*(WORD *)buff = SECTORSIZE;
		break;
	case (BYTE)GET_SECTOR_COUNT:
		*(DWORD *)buff = SECTORCNT;
		break;
	default:
		res = RES_PARERR;
		break;
	}
#endif

	return res;
}

/******************************************************************************/
/**
*
* This function is User Provided Timer Function for FatFs module
*
* @param	None
*
* @return	DWORD
*
* @note		None
*
****************************************************************************/
//Return Value
//Currnet local time shall be returned as bit-fields packed into a DWORD value. The bit fields are as follows:
//
//bit31:25
//Year origin from the 1980 (0..127, e.g. 37 for 2017)
//bit24:21
//Month (1..12)
//bit20:16
//Day of the month (1..31)
//bit15:11
//Hour (0..23)
//bit10:5
//Minute (0..59)
//bit4:0
//Second / 2 (0..29, e.g. 25 for 50)

//	fatfs timestamp from 1980-01-01 00:00:00
//	7		4		5		5		6		5
//	D31~D25	D24~D21	D20~D16	D15~D11	D10~D5	D4~D0
//	year	month	day		hour	minute	second/2
//	00~127	1~12	1~31	0~23	00~59	00~29
//  1980	1		1		0		0		0

//	dcm pcf8563 timestamp from 2000-01-01 00:00:00
//	6		4		5		5		6		6
//	D31~D26	D25~D22	D21~D17	D16~D12	D11~D6	D5~D0
//	year	month	day		hour	minute	second
//	00~63	1~12	1~31	0~23	00~59	00~29
//  2000	1		1		0		0		0

DWORD get_fattime (void)
{
//	DateTime32 dt;
//	dt = DateTime;
//	return	  ((DWORD)(dt.YearFrom2000 + 20U) << 25U)	/* Fixed to Jun. 18, 2018 */
//			| ((DWORD) dt.Month << 21)
//			| ((DWORD) dt.Day << 16)
//			| ((DWORD) dt.Hour << 11)
//			| ((DWORD) dt.Minute << 5)
//			| ((DWORD) dt.Second >> 1);

//	DateTime32 dt32;
//	u32 dt;
//	dt32 = DateTime;
//	dt = dt32.Value >> 1;
//	dt = (dt & ~0xfe000000) | ((dt32.YearFrom2000 + 20) << 25);
//	return dt;

	u32 fat_dt_u32 = ((DWORD)((DateTime.Value >> 1)+(20 << 25)));
	return fat_dt_u32;
}

/*****************************************************************************/
/**
*
* Reads the drive
* In case of SD, it reads the SD card using ADMA2 in polled mode.
*
* @param	pdrv - Drive number
* @param	*buff - Pointer to the data to be written
* @param	sector - Sector address
* @param	count - Sector count
*
* @return
*		RES_OK		Read successful
*		STA_NOINIT	Drive not initialized
*		RES_ERROR	Read not successful
*
* @note
*
******************************************************************************/
int wrcnt = 0;

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
	wrcnt++;
	DSTATUS s;
#ifdef FILE_SYSTEM_INTERFACE_SD
	s32 Status;
	DWORD LocSector = sector;
#endif
	configASSERT(pdrv == 0);
	s = disk_status(pdrv);
	if ((s & STA_NOINIT) != 0U) {
		return RES_NOTRDY;
	}
	if (count == 0U) {
		return RES_PARERR;
	}

#ifdef FILE_SYSTEM_INTERFACE_SD
	if(0 == mmc_bwrite(sddrv, LocSector, count, buff))
	{
		return RES_ERROR;
	}
//	/* Convert LBA to byte address if needed */
//	if ((SdInstance[pdrv].HCS) == 0U) {
//		LocSector *= (DWORD)XSDPS_BLK_SIZE_512_MASK;
//	}
//
//	Status  = XSdPs_WritePolled(&SdInstance[pdrv], (u32)LocSector, count, buff);
//	if (Status != XST_SUCCESS) {
//		return RES_ERROR;
//	}

#endif

#ifdef FILE_SYSTEM_INTERFACE_RAM
	memcpy(dataramfs + (sector * SECTORSIZE), buff, count * SECTORSIZE);
#endif

	return RES_OK;
}

//void speedTest()
//{
//	int i = 0, j;
////	for(i = 0; i < 10; i++)
////	{
//		for(j = 0; j < 384; j++)	// (w12k,r12k,w12k,r12k)*384=18M
//		{
//			mmc_bwrite(sddrv, 1000000 + j*24, 24, (u8 *)0x88000000);
//			mmc_bread(sddrv, 1000000 + j*24, 24, (u8 *)0x88000000);
//			mmc_bwrite(sddrv, 1000000 + j*24, 24, (u8 *)0x88003000);
//			mmc_bread(sddrv, 1000000 + j*24, 24, (u8 *)0x88003000);
//		}
////	}
//}
