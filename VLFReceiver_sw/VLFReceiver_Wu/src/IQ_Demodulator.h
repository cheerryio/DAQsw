/*
 * IQ_Demodulator.h
 *
 *  Created on: Mar 24, 2021
 *      Author: WuXJ
 */

#ifndef IQ_DEMODULATOR_H_
#define IQ_DEMODULATOR_H_

/*************************************************
 * IQ_Demodulator.sv IP description
 */
/*
*    Author: WuXj
*    Date: 2021.03.20
*    Description:
*        this file implement functions followed:
*        (1) input signal with 24 bits @512k sample rate;
*        (2) IQ demodlate with OrthDDS#(32, 12, 14)
*        (3) CIC downsampler of 16 into @32k
*        (4) FIR LPF of 2k \_ 8k LPF @ 32k
*        (5) downsampler of 2 into @16k and output
*
*    Regs:
*    slv_reg0:   DDS frequency word = 2^32 / 100e6 * freq;
*    slv_reg1:   DDS phase     word = 2^32 / 360  * phase(in degrees);
*    slv_reg2:   frame counts to output tlast;
*    slv_reg3:   [0] bypass demodlation
*                [1] bypass CIC
*                [2] bypass FIR
*/

#include "xparameters.h"

#define cal_freq(freq) 			((u32)( double(1ll<<32) / 100000000.0 * double(freq)))
#define set_freq(freq) 			Xil_Out32(XPAR_ADC0_IQ_DEMODULATOR_0_S00_AXI_BASEADDR, cal_freq(freq));\
								Xil_Out32(XPAR_ADC1_IQ_DEMODULATOR_0_S00_AXI_BASEADDR, cal_freq(freq))
#define cal_phase(pha) 			((u32)( double(1ll<<32) * / 360.0 * double(pha)))
#define set_phase(pha) 			Xil_Out32(XPAR_ADC0_IQ_DEMODULATOR_0_S00_AXI_BASEADDR + 4, cal_phase(pha));\
								Xil_Out32(XPAR_ADC1_IQ_DEMODULATOR_0_S00_AXI_BASEADDR + 4, cal_phase(pha))
#define set_tlast_count(cnt) 	Xil_Out32(XPAR_ADC0_IQ_DEMODULATOR_0_S00_AXI_BASEADDR + 8, cnt);\
								Xil_Out32(XPAR_ADC1_IQ_DEMODULATOR_0_S00_AXI_BASEADDR + 8, cnt)



#endif /* IQ_DEMODULATOR_H_ */
