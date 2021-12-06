#include "common.h"

XilGpio* gpio_adc_ctrl;
XilPsDma* ps_dma;

void initPeriph()
{
    gpio_adc_ctrl=new XilGpio(XPAR_ADC_CTRL_DEVICE_ID);
    XGpio_SetDataDirection(gpio_adc_ctrl->returnGpio(),1,0x00000000);
    XGpio_SetDataDirection(gpio_adc_ctrl->returnGpio(),2,0x00000000);
    ps_dma=new XilPsDma(XPAR_PS7_DMA_S_DEVICE_ID);
    gpio_adc_ctrl->XilGpioDiscreteWrite(1,0);
    gpio_adc_ctrl->XilGpioDiscreteWrite(2,0);
}
