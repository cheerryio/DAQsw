/*
 * bl.c
 *
 *  Modified on: Mar 30, 2015
 *      Author: loywong
 */

/*
 * bootloader.c
 *
 *  Created on: Mar 9, 2013
 *      Author: yzhang
 */

/***************************** Include Files *********************************/

#include "xparameters.h"	/* EDK generated parameters */
#include "xspi.h"		/* SPI device driver */


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#warning "Please correct the 8 definitions below!"

#define SOURCE_SPI_FLASH_DEVICE_ID	XPAR_PERIPHERALS_AXI_QUAD_SPI_FLASH_DEVICE_ID
#define DESTINATION_DDR3_BASEADDR	XPAR_MEMORIES_MIG_7SERIES_0_BASEADDR

#define SPI_FLASH_SELECT 			0x01
#define SPI_FLASH_PAGE_SIZE			256

// for XC7A35T with 16MB flash
#define VECTOR_SECTION_START_ADDR	0x27FF00
#define REST_SECTION_START_ADDR		0x280000

#define VECTOR_SECTION_BYTE_NUM		40
#define REST_SECTION_BYTE_NUM		222236 //190992

/*
 * Definitions of the commands shown in this example.
 */
#define COMMAND_WRITE_REGS			0x01
#define COMMAND_PAGE_PROGRAM		0x02 /* Page Program command */
#define COMMAND_QUAD_WRITE			0x32 /* Quad Input Fast Program */
#define COMMAND_RANDOM_READ			0x03 /* Random read command */
#define COMMAND_DUAL_READ			0x3B /* Dual Output Fast Read */
#define COMMAND_DUAL_IO_READ		0xBB /* Dual IO Fast Read */
#define COMMAND_QUAD_READ			0x6B /* Quad Output Fast Read */
#define COMMAND_QUAD_IO_READ		0xEB /* Quad IO Fast Read */
#define	COMMAND_WRITE_ENABLE		0x06 /* Write Enable command */
#define COMMAND_SECTOR_ERASE		0xD8 /* Sector Erase command */
#define COMMAND_BULK_ERASE			0xC7 /* Bulk Erase command */
#define COMMAND_STATUSREG_READ		0x05 /* Status read command */

/**
 * This definitions specify the EXTRA bytes in each of the command
 * transactions. This count includes Command byte, address bytes and any
 * don't care bytes needed.
 */
#define READ_WRITE_EXTRA_BYTES		4 /* Read/Write extra bytes */
#define	WRITE_ENABLE_BYTES		1 /* Write Enable bytes */
#define SECTOR_ERASE_BYTES		4 /* Sector erase extra bytes */
#define BULK_ERASE_BYTES		1 /* Bulk erase extra bytes */
#define STATUS_READ_BYTES		2 /* Status read bytes count */
#define STATUS_WRITE_BYTES		2 /* Status write bytes count */

/*
 * Byte Positions.
 */
#define BYTE1				0 /* Byte 1 position */
#define BYTE2				1 /* Byte 2 position */
#define BYTE3				2 /* Byte 3 position */
#define BYTE4				3 /* Byte 4 position */
#define BYTE5				4 /* Byte 5 position */
#define BYTE6				5 /* Byte 6 position */
#define BYTE7				6 /* Byte 7 position */
#define BYTE8				7 /* Byte 8 position */

/*
 * The following definitions specify the number of dummy bytes to ignore in the
 * data read from the flash, through various Read commands. This is apart from
 * the dummy bytes returned in reponse to the command and address transmitted.
 */
/*
 * After transmitting Dual Read command and address on DIO0,the quad spi device
 * configures DIO0 and DIO1 in input mode and receives data on both DIO0 and
 * DIO1 for 8 dummy clock cycles. So we end up with 16 dummy bits in DRR. The
 * same logic applies Quad read command, so we end up with 4 dummy bytes in
 * that case.
 */
#define DUAL_READ_DUMMY_BYTES		2
#define QUAD_READ_DUMMY_BYTES		4

#define DUAL_IO_READ_DUMMY_BYTES	2
#define QUAD_IO_READ_DUMMY_BYTES	5

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int SpiFlashRead(XSpi *SpiPtr, u32 Addr, u32 ByteCount, u8 ReadCmd);
int SpiFlashWriteEnable(XSpi *SpiPtr);
int (*boot_app) (void);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */

static XSpi Spi;


/*
 * Buffers used during read and write transactions.
 */
static u8 ReadBuffer[SPI_FLASH_PAGE_SIZE + READ_WRITE_EXTRA_BYTES + 4];
static u8 WriteBuffer[SPI_FLASH_PAGE_SIZE + READ_WRITE_EXTRA_BYTES];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* Main function to copy from flash to DDR3.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/

//=============================================================================
//void test(XSpi *SpiPtr)
//{
//	int Status;
//
//	WriteBuffer[BYTE1] = COMMAND_WRITE_ENABLE;
//
//	Status = XSpi_Transfer(SpiPtr, WriteBuffer, NULL, WRITE_ENABLE_BYTES);
//
//	if(Status != XST_SUCCESS)
//	{
//		return XST_FAILURE;
//	}
//
//	return XST_SUCCESS;
//}
//-----------------------------------------------------------------------------

int main(void)
{
	int Status;
	int i;
	int k;
	u8* destination_location = (u8 *)DESTINATION_DDR3_BASEADDR;
	u8* reset_location = (u8 *)0x0;
	u32 Index;
	XSpi_Config *ConfigPtr;	/* Pointer to Configuration data */


	/*
	 * Initialize the SPI driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h.
	 */
	ConfigPtr = XSpi_LookupConfig(SOURCE_SPI_FLASH_DEVICE_ID);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XSpi_CfgInitialize(&Spi, ConfigPtr,
				  ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the SPI device as a master and in manual slave select mode such
	 * that the slave select signal does not toggle for every byte of a
	 * transfer, this must be done before the slave select is set.
	 */
	Status = XSpi_SetOptions(&Spi, XSP_MASTER_OPTION |
				 XSP_MANUAL_SSELECT_OPTION);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Select the quad flash device on the SPI bus, so that it can be
	 * read using the SPI bus.
	 */
	Status = XSpi_SetSlaveSelect(&Spi, SPI_FLASH_SELECT);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the SPI driver so that the device are enabled.
	 */
	XSpi_Start(&Spi);

	/*
	 * Disable Global interrupt to use polled mode operation
	 */
	XSpi_IntrGlobalDisable(&Spi);

	/*
	 * Perform the Write Enable operation.
	 */
	Status = SpiFlashWriteEnable(&Spi);
    if(Status != XST_SUCCESS) {
	return XST_FAILURE;
    }

    //====================== test =======================

    //---------------------------------------------------

	/*
	 * Clear the read Buffer.
	 */
	for(Index = 0; Index < SPI_FLASH_PAGE_SIZE + READ_WRITE_EXTRA_BYTES; Index++) {
		ReadBuffer[Index] = 0x0;
	}

	/*
	 * Read the rest_section data from flash.
	 */
	for(i=0; i<(REST_SECTION_BYTE_NUM/16)+1; i++)
	{
	    Status = SpiFlashRead(&Spi, (REST_SECTION_START_ADDR+16*i), 16, COMMAND_RANDOM_READ);
	    if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	    }
	    for(k=0; k<16; k++)
		*destination_location++ = ReadBuffer[k+4];
	}
//    xil_printf("non-vector sections load succeed!\r\n");
	/*
	 * Read the vector_section data from flash.
	 */

	for(i=0; i<(VECTOR_SECTION_BYTE_NUM/16)+1; i++)
	{
		Status = SpiFlashRead(&Spi, (VECTOR_SECTION_START_ADDR+16*i), 16, COMMAND_RANDOM_READ);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		for(k=0; k<16; k++)
			*reset_location++ = ReadBuffer[k+4];
	}

//	xil_printf("user application load succeed!\r\n");

	boot_app = (int (*) (void)) DESTINATION_DDR3_BASEADDR;
	boot_app();

	return XST_SUCCESS;
}





/*****************************************************************************/
/**
*
* This function reads the data from the Numonyx Serial Flash Memory
*
* @param	SpiPtr is a pointer to the instance of the Spi device.
* @param	Addr is the starting address in the Flash Memory from which the
*		data is to be read.
* @param	ByteCount is the number of bytes to be read.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int SpiFlashRead(XSpi *SpiPtr, u32 Addr, u32 ByteCount, u8 ReadCmd)
{
	int Status;

	/*
	 * Prepare the WriteBuffer.
	 */
	WriteBuffer[BYTE1] = ReadCmd;
	WriteBuffer[BYTE2] = (u8) (Addr >> 16);
	WriteBuffer[BYTE3] = (u8) (Addr >> 8);
	WriteBuffer[BYTE4] = (u8) Addr;

	/*
	 * Initiate the Transfer.
	 */
	Status = XSpi_Transfer( SpiPtr, WriteBuffer, ReadBuffer,
				(ByteCount + READ_WRITE_EXTRA_BYTES));
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int SpiFlashWriteEnable(XSpi *SpiPtr)
{
	int Status;

	/*
	 * Wait while the Flash is busy.
	 */



	/*
	 * Prepare the WriteBuffer.
	 */
	WriteBuffer[BYTE1] = COMMAND_WRITE_ENABLE;

	/*
	 * Initiate the Transfer.
	 */

	Status = XSpi_Transfer(SpiPtr, WriteBuffer, NULL,
				WRITE_ENABLE_BYTES);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Transfer is complete and check if there are any errors
	 * in the transaction..
	 */


	return XST_SUCCESS;
}
