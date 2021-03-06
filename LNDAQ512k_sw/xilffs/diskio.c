/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/

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
* 3.9   mn   04/18/18 Resolve build warnings for xilffs library
*       mn   07/06/18 Fix Cppcheck and Doxygen warnings
* 4.2   mn   08/16/19 Initialize Status variables with failure values
*       mn   09/25/19 Check if the SD is powered on or not in disk_status()
* 4.3   mn   02/24/20 Remove unused macro defines
*       mn   04/08/20 Set IsReady to '0' before calling XSdPs_CfgInitialize
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
//#include "xsdps.h"		/* SD device driver */

#include "../sdc_ctrl/mmc.h"		/* SD device driver */
struct mmc * ocsdc_mmc_init(int base_addr, int clk_freq);

#endif
#include "sleep.h"
#include "xil_printf.h"

#include "xil_printf.h"
#include "mb_interface.h"

#include "../TskHif/TskHif.h"

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

//	DSTATUS s = Stat[pdrv];
//#ifdef FILE_SYSTEM_INTERFACE_SD
//	u32 StatusReg;
//	u32 DelayCount = 0;
//
//		if (SdInstance[pdrv].Config.BaseAddress == (u32)0) {
//				XSdPs_Config *SdConfig;
//
//				SdConfig = XSdPs_LookupConfig((u16)pdrv);
//				if (NULL == SdConfig) {
//					s |= STA_NOINIT;
//					return s;
//				}
//
//				BaseAddress = SdConfig->BaseAddress;
//				CardDetect = SdConfig->CardDetect;
//				WriteProtect = SdConfig->WriteProtect;
//
//				HostCntrlrVer[pdrv] = (u8)(XSdPs_ReadReg16(BaseAddress,
//						XSDPS_HOST_CTRL_VER_OFFSET) & XSDPS_HC_SPEC_VER_MASK);
//				if (HostCntrlrVer[pdrv] == XSDPS_HC_SPEC_V3) {
//					SlotType[pdrv] = XSdPs_ReadReg(BaseAddress,
//							XSDPS_CAPS_OFFSET) & XSDPS_CAPS_SLOT_TYPE_MASK;
//				} else {
//					SlotType[pdrv] = 0;
//				}
//		}
//
//		/* If SD is not powered up then mark it as not initialized */
//		if ((XSdPs_ReadReg8((u32)BaseAddress, XSDPS_POWER_CTRL_OFFSET) &
//			XSDPS_PC_BUS_PWR_MASK) == 0U) {
//			s |= STA_NOINIT;
//		}
//
//		StatusReg = XSdPs_GetPresentStatusReg((u32)BaseAddress);
//		if (SlotType[pdrv] != XSDPS_CAPS_EMB_SLOT) {
//			if (CardDetect) {
//				while ((StatusReg & XSDPS_PSR_CARD_INSRT_MASK) == 0U) {
//					if (DelayCount == 500U) {
//						s = STA_NODISK | STA_NOINIT;
//						goto Label;
//					} else {
//						/* Wait for 10 msec */
//						usleep(SD_CD_DELAY);
//						DelayCount++;
//						StatusReg = XSdPs_GetPresentStatusReg((u32)BaseAddress);
//					}
//				}
//			}
//			s &= ~STA_NODISK;
//			if (WriteProtect) {
//					if ((StatusReg & XSDPS_PSR_WPS_PL_MASK) == 0U){
//						s |= STA_PROTECT;
//						goto Label;
//					}
//			}
//			s &= ~STA_PROTECT;
//		} else {
//			s &= ~STA_NODISK & ~STA_PROTECT;
//		}
//
//
//Label:
//		Stat[pdrv] = s;
//#endif
//
//		return s;
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
//	DSTATUS s;
//#ifdef FILE_SYSTEM_INTERFACE_SD
//	s32 Status = XST_FAILURE;
//	XSdPs_Config *SdConfig;
//#endif
//
//	s = disk_status(pdrv);
//	if ((s & STA_NODISK) != 0U) {
//		return s;
//	}
//
//	/* If disk is already initialized */
//	if ((s & STA_NOINIT) == 0U) {
//		return s;
//	}
//
//#ifdef FILE_SYSTEM_INTERFACE_SD
//	if (CardDetect) {
//			/*
//			 * Card detection check
//			 * If the HC detects the No Card State, power will be cleared
//			 */
//			while(!((XSDPS_PSR_CARD_DPL_MASK |
//					XSDPS_PSR_CARD_STABLE_MASK |
//					XSDPS_PSR_CARD_INSRT_MASK) ==
//					( XSdPs_GetPresentStatusReg((u32)BaseAddress) &
//					(XSDPS_PSR_CARD_DPL_MASK |
//					XSDPS_PSR_CARD_STABLE_MASK |
//					XSDPS_PSR_CARD_INSRT_MASK))));
//	}
//
//	/*
//	 * Initialize the host controller
//	 */
//	SdConfig = XSdPs_LookupConfig((u16)pdrv);
//	if (NULL == SdConfig) {
//		s |= STA_NOINIT;
//		return s;
//	}
//
//	SdInstance[pdrv].IsReady = 0U;
//
//	Status = XSdPs_CfgInitialize(&SdInstance[pdrv], SdConfig,
//					SdConfig->BaseAddress);
//	if (Status != XST_SUCCESS) {
//		s |= STA_NOINIT;
//		return s;
//	}
//
//	Status = XSdPs_CardInitialize(&SdInstance[pdrv]);
//	if (Status != XST_SUCCESS) {
//		s |= STA_NOINIT;
//		return s;
//	}
//
//
//	/*
//	 * Disk is initialized.
//	 * Store the same in Stat.
//	 */
//	s &= (~STA_NOINIT);
//
//	Stat[pdrv] = s;
//#endif
//
//#ifdef FILE_SYSTEM_INTERFACE_RAM
//	/* Assign RAMFS address value from xparameters.h */
//	dataramfs = (char *)RAMFS_START_ADDR;
//
//	/* Clearing No init Status for RAM */
//	s &= (~STA_NOINIT);
//	Stat[pdrv] = s;
//#endif
//
//	return s;
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
//	DSTATUS s;
//#ifdef FILE_SYSTEM_INTERFACE_SD
//	s32 Status = XST_FAILURE;
//	DWORD LocSector = sector;
//#endif
//
//	s = disk_status(pdrv);
//
//	if ((s & STA_NOINIT) != 0U) {
//		return RES_NOTRDY;
//	}
//	if (count == 0U) {
//		return RES_PARERR;
//	}
//
//#ifdef FILE_SYSTEM_INTERFACE_SD
//	/* Convert LBA to byte address if needed */
//	if ((SdInstance[pdrv].HCS) == 0U) {
//		LocSector *= (DWORD)XSDPS_BLK_SIZE_512_MASK;
//	}
//
//	Status  = XSdPs_ReadPolled(&SdInstance[pdrv], (u32)LocSector, count, buff);
//	if (Status != XST_SUCCESS) {
//		return RES_ERROR;
//	}
//#endif
//
//#ifdef FILE_SYSTEM_INTERFACE_RAM
//	memcpy(buff, dataramfs + (sector * SECTORSIZE), count * SECTORSIZE);
//#endif
//
//#if !defined(FILE_SYSTEM_INTERFACE_SD) && !defined(FILE_SYSTEM_INTERFACE_RAM)
//	(void)buff;
//	(void)sector;
//#endif
//
//    return RES_OK;
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
//	DRESULT res = RES_ERROR;
//
//#ifdef FILE_SYSTEM_INTERFACE_SD
//	void *LocBuff = buff;
//	if ((disk_status(pdrv) & STA_NOINIT) != 0U) {	/* Check if card is in the socket */
//		return RES_NOTRDY;
//	}
//
//	switch (cmd) {
//		case (BYTE)CTRL_SYNC :	/* Make sure that no pending write process */
//			res = RES_OK;
//			break;
//
//		case (BYTE)GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
//			(*((DWORD *)(void *)LocBuff)) = (DWORD)SdInstance[pdrv].SectorCount;
//			res = RES_OK;
//			break;
//
//		case (BYTE)GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
//			(*((DWORD *)((void *)LocBuff))) = ((DWORD)128);
//			res = RES_OK;
//			break;
//
//		default:
//			res = RES_PARERR;
//			break;
//	}
//#endif
//
//#ifdef FILE_SYSTEM_INTERFACE_RAM
//	switch (cmd) {
//	case (BYTE)CTRL_SYNC:
//		res = RES_OK;
//		break;
//	case (BYTE)GET_BLOCK_SIZE:
//		*(WORD *)buff = BLOCKSIZE;
//		res = RES_OK;
//		break;
//	case (BYTE)GET_SECTOR_SIZE:
//		*(WORD *)buff = SECTORSIZE;
//		res = RES_OK;
//		break;
//	case (BYTE)GET_SECTOR_COUNT:
//		*(DWORD *)buff = SECTORCNT;
//		res = RES_OK;
//		break;
//	default:
//		res = RES_PARERR;
//		break;
//	}
//#endif
//
//#if !defined(FILE_SYSTEM_INTERFACE_SD) && !defined(FILE_SYSTEM_INTERFACE_RAM)
//	(void)pdrv;
//	(void)cmd;
//	(void)buff;
//#endif
//
//	return res;
}

/******************************************************************************/
/**
*
* This function is User Provided Timer Function for FatFs module
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
//DWORD get_fattime (void)
//{
//	return	((DWORD)(2010U - 1980U) << 25U)	/* Fixed to Jan. 1, 2010 */
//		| ((DWORD)1 << 21)
//		| ((DWORD)1 << 16)
//		| ((DWORD)0 << 11)
//		| ((DWORD)0 << 5)
//		| ((DWORD)0 >> 1);
//}

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
//	DSTATUS s;
//#ifdef FILE_SYSTEM_INTERFACE_SD
//	s32 Status = XST_FAILURE;
//	DWORD LocSector = sector;
//#endif
//
//	s = disk_status(pdrv);
//	if ((s & STA_NOINIT) != 0U) {
//		return RES_NOTRDY;
//	}
//	if (count == 0U) {
//		return RES_PARERR;
//	}
//
//#ifdef FILE_SYSTEM_INTERFACE_SD
//	/* Convert LBA to byte address if needed */
//	if ((SdInstance[pdrv].HCS) == 0U) {
//		LocSector *= (DWORD)XSDPS_BLK_SIZE_512_MASK;
//	}
//
//	Status  = XSdPs_WritePolled(&SdInstance[pdrv], (u32)LocSector, count, buff);
//	if (Status != XST_SUCCESS) {
//		return RES_ERROR;
//	}
//
//#endif
//
//#ifdef FILE_SYSTEM_INTERFACE_RAM
//	memcpy(dataramfs + (sector * SECTORSIZE), buff, count * SECTORSIZE);
//#endif
//
//#if !defined(FILE_SYSTEM_INTERFACE_SD) && !defined(FILE_SYSTEM_INTERFACE_RAM)
//	(void)buff;
//	(void)sector;
//#endif
//
//	return RES_OK;
}
