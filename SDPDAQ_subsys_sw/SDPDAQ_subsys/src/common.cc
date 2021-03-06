#include "common.h"
XilIntc* 			intc;
XilGpio*			gpioStartADC;
XilGpio*			gpioResetFifo;
XilGpio*			gpioAdcDmaLastM;
XilAxiDma*			adcDataDma;
XillUartps*			uartTransSubs;
XillUartps*			uartGps;
secCnt*				theSecCnt;
extern void adcDmaS2MMIsr(void* );
void initPeriph()
{
	intc = new XilIntc(XPAR_SCUGIC_0_DEVICE_ID);

	gpioStartADC = new XilGpio(XPAR_ADC_HIER_AXI_GPIO_START_DEVICE_ID);
	gpioStartADC->XilGpioDiscreteWrite(1, 0);
	gpioResetFifo = new XilGpio(XPAR_ADC_HIER_AXI_GPIO_RESETFIFO_DEVICE_ID);
	gpioResetFifo->XilGpioDiscreteWrite(1, 0);
	gpioAdcDmaLastM = new XilGpio(XPAR_ADC_HIER_AXI_GPIO_LASTMODULO_DEVICE_ID);

	adcDataDma = new XilAxiDma(XPAR_ADC_HIER_AXI_DMA_ADC_DEVICE_ID, 0, XPAR_FABRIC_AXIDMA_0_VEC_ID, intc, NULL, NULL);
}



