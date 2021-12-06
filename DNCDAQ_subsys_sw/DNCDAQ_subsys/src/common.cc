#include "common.h"

XilGpio* gpio_adc_ctrl;
XilAxiDma* axi_dma;
Eth* net;

void initPeriph()
{
    gpio_adc_ctrl=new XilGpio(XPAR_ADC_CTRL_DEVICE_ID);
    XGpio_SetDataDirection(gpio_adc_ctrl->returnGpio(),1,0x00000000);
    XGpio_SetDataDirection(gpio_adc_ctrl->returnGpio(),2,0x11111111);
    gpio_adc_ctrl->XilGpioDiscreteWrite(1, 0);
    
    axi_dma=new XilAxiDma(XPAR_AXI_DMA_0_DEVICE_ID,0U,XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR,NULL,NULL);
    net=new Eth(XPAR_PS7_ETHERNET_0_BASEADDR,CLIENT_IP_ADDR,CLIENT_NETMASK,CLIENT_GW);
}
