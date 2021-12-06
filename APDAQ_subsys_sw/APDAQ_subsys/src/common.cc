#include "common.h"

XilIntc* intc;
XilGpio* gpio_adc_en;
XilAxiDma* axi_dma;

void initPeriph()
{
    intc=new XilIntc(XPAR_SCUGIC_0_DEVICE_ID);
    gpio_adc_en=new XilGpio(XPAR_ADS127L01_EN_DEVICE_ID);
    gpio_adc_en->XilGpioDiscreteWrite(1, 0);
    axi_dma=new XilAxiDma(XPAR_AXI_DMA_0_DEVICE_ID,0U,XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR,intc,NULL,NULL);
}
