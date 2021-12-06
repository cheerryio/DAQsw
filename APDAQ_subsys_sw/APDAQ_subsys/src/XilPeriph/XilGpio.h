#ifndef _XILGPIO_H_
#define _XILGPIO_H_
#include "xgpio.h"
#include "xil_printf.h"
typedef enum{
	OUTPUT,
	INPUT
}XilGpioDir_t;

class XilGpio{
private:
	u16 			deviceId;
	XGpio 			gpio;
	XGpio_Config 	*gpioCfg;
	XilGpioDir_t 	dir;

public:
	XilGpio(
			u16 GpioId
	);

	int 	XilGpioInit();
	u32 	XilGpioDiscreteRead(unsigned Channel);
	void 	XilGpioDiscreteWrite(unsigned Channel, u32 Mask);
	void 	XilGpioSetDataDirection(unsigned Channel, unsigned bit, XilGpioDir_t gpioDir);
	XilGpioDir_t 	XilGpioGetDataDirection(unsigned Channel);
	void 	XilGpioDiscreteSet(unsigned Channel, u32 Mask);
	void 	XilGpioDiscreteClear(unsigned Channel, u32 Mask);
};



#endif
