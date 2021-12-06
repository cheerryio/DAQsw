#include "XilGpio.h"


XilGpio::XilGpio(u16 GpioId)
{
	deviceId = GpioId;
	u32 Status = 0;

	Status = XilGpioInit();

}


int XilGpio::XilGpioInit()
{
	int Status;

	Status = XGpio_Initialize(&gpio, deviceId);
	if (Status != XST_SUCCESS) {
		xil_printf("TREADY GPIO init failed - %x\r\n", Status);
		return Status;
	}
	return XST_SUCCESS;
}

u32 XilGpio::XilGpioDiscreteRead(unsigned Channel)
{
	return XGpio_DiscreteRead(&gpio, Channel);
}
void XilGpio::XilGpioDiscreteWrite(unsigned Channel, u32 Mask)
{
	XGpio_DiscreteWrite(&gpio, Channel, Mask);
}
void XilGpio::XilGpioSetDataDirection(unsigned Channel, unsigned bit, XilGpioDir_t gpioDir)
{
	XGpio_SetDataDirection(&gpio, Channel,(u32)gpioDir << bit);
}
XilGpioDir_t XilGpio::XilGpioGetDataDirection(unsigned Channel)
{
	return (XilGpioDir_t)(XGpio_GetDataDirection(&gpio, Channel));
}
void XilGpio::XilGpioDiscreteSet(unsigned Channel, u32 Mask)
{
	XGpio_DiscreteSet(&gpio, Channel, Mask);
}
void XilGpio::XilGpioDiscreteClear(unsigned Channel, u32 Mask)
{
	XGpio_DiscreteClear(&gpio, Channel, Mask);
}
