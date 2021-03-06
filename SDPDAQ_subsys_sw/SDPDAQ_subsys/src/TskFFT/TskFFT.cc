#include "TskFFT.h"
#include "../TskSD/SD.h"

using namespace LoyMath;

const int tskFFTPrio = 7;
const int tskFFTStkSize = 16384;
TaskHandle_t tskFFTHandle;
static void tskFFT(void *arg);

extern XilGpio* gpioStartADC;
extern XilGpio* gpioResetFifo;
extern XilGpio* gpioAdcDmaLastM;

const float hanning_correction=1.6392f;
const float w=ADC_SAMPLE_RATE/N;
FFT<float,14> fft __attribute__((aligned(64)));
Cplx<float> complex_x[N] __attribute__((aligned(64)));
Cplx<float> complex_y[N] __attribute__((aligned(64)));
float hanning_window[N] __attribute__((aligned(64)));
int fixed_x[N] __attribute__((aligned(64)));
float a[N] __attribute__((aligned(64)));
float b[N/2] __attribute__((aligned(64)));
float freq[N/2] __attribute__((aligned(64)));
float Psd_avg[N/2] __attribute__((aligned(64)));
float Psd_peek[N/2] __attribute__((aligned(64)));
float fft_result[N/2] __attribute__((aligned(64)));

extern u8 fftbuf[FFTBUF_NUM][TOTAL_FFT_BYTE] __attribute__((aligned(128)));
extern ctrlReg_t ctrlReg;
extern volatile bool doingfft;
extern volatile unsigned int fftbuf_wr_p;

void writeFFTdata(){
    DIR dir;
    FIL wf;
	int sts;
	unsigned int bw;
    char fileName[64];
	char line[64];
    tm cur_time;
    cur_time=ctrlReg.curTime;
    sprintf(fileName, "FFT-%4d-%d-%d-%d-%d.csv", cur_time.tm_year + 1900, cur_time.tm_mon + 1, cur_time.tm_mday,
														cur_time.tm_hour, cur_time.tm_min);
	f_opendir(&dir,"/");
    f_open(&wf,fileName,FA_CREATE_ALWAYS | FA_WRITE);
    //sprintf(line,"seq,freq,avg,peek\r\n");
	//sts=f_write(wf,line,strlen(line),&bw);
	//configASSERT(sts==FR_OK && bw==strlen(line));
	for(int i=0;i<N/2;i++){
		sprintf(line,"%d,%f,%f,%f\r\n",i,freq[i],Psd_avg[i],Psd_peek[i]);
		sts=f_write(&wf,line,strlen(line),&bw);
		configASSERT(sts==FR_OK && bw==strlen(line));
	}
    sts=f_sync(&wf);
    sts=f_close(&wf);
}

static FFT<float,14> myfft;
static Cplx<float> complex_test_x[N];
static Cplx<float> complex_test_y[N];
static float test_x[N];
static void tskFFTSimple(void *arg){
	printf("in tskFFT simple\n\r");
    while(1){
        float power=0.0f;
        float powerx=0.0f,powery=0.0f;
        float m;
        memset((void*)complex_test_x,0,(N)*sizeof(Cplx<float>));
        memset((void*)complex_test_y,0,(N)*sizeof(Cplx<float>));
        memset((void*)test_x,0,N*sizeof(float));
        float sum=0;
        for(int i=0;i<N;i++){
            test_x[i]=sin(2*3.1415926535f*1000.0f*i/N)*2.5f;
            sum+=test_x[i];
            complex_test_x[i]=Cplx<float>(test_x[i],0.0f);
        }
        //myfft.Trans(complex_test_x,complex_test_y,-1,1);
        for(int i=0;i<N;i++){
        	complex_test_y[i]=complex_test_x[i];
        }
        for(int i=0;i<N/2;i++){
            m=(complex_test_x[i].real*complex_test_x[i].real+complex_test_x[i].imag*complex_test_x[i].imag)*4.0f;
            powerx+=m;
        }
        printf("powerx=%f, powery=%f, sum=%f\n\r",powerx,powery,sum);
    }

    vTaskDelete(NULL);
}

static void tskFFT(void *arg){
    unsigned int round=0;
	float power=0.0f;
    float total_power=0.0f;
    int t;
    int *t_ptr=(int*)&t;
    float x;
    float m;
    u8 *p;
    while(1){
        if(!doingfft){
            if(round>0){
                for(int i=0;i<N/2;i++){
                    Psd_avg[i]=Psd_avg[i]/round*1.0f;
                }
                total_power/=round;
                writeFFTdata();
                printf("FFT complete. round %d, power:%d\n\r",round,(int)total_power);
            }
            vTaskSuspend(NULL);
            round=0;
            total_power=0.0f;
            memset((void*)&Psd_avg,0,(N/2)*sizeof(float));
            memset((void*)&Psd_peek,0,(N/2)*sizeof(float));
        }else{
            power=0.0f;
            p=fftbuf[fftbuf_wr_p];
            Xil_DCacheInvalidateRange((UINTPTR)p,TOTAL_FFT_BYTE);
            for(int i=0;i<N;i++){
                t=0;
                memcpy(t_ptr,p+3*i,3);
                fixed_x[i]=t&0x00800000?t|0xff000000:t;
            }
            for(int i=0;i<N;i++){
                x=fixed_x[i]*1.0f/powf(2.0f,23.0f)*12.5f;
                x=x*hanning_window[i];
                complex_x[i]=Cplx<float>(x,0.0f);
            }
            fft.Trans(complex_x,complex_y,-1,1);
            for(int i=0;i<N/2;i++){
                if(i==0 || i==(N/2-1))
                	m=complex_y[i].Power();
                else
                    m=complex_y[i].Power()*4.0f;
                m=(1.0f/2.0f*2.6870f/w)*m;
                fft_result[i]=m;
                power+=m*w;
            }
            printf("total_power=%d,power=%f\n\r",(int)total_power,power);
            if(power<total_power/round*2 || round==0){
                total_power+=power;
                for(int i=0;i<N/2;i++){
                    Psd_avg[i]+=fft_result[i];
                    Psd_peek[i]=std::max<float>(Psd_peek[i],fft_result[i]);
                }
                round++;
            }else{
            	printf("wrong,wrong,power=%d\n\r",(int)power);
            	printf("wrong,wrong\n\r");
            	printf("wrong,wrong\n\r");
            	printf("wrong,wrong\n\r");
            }
        }
    }

    vTaskDelete(NULL);
}

void tskFFTInit(){
    int ret;

    // hanning window
    for(int i=0;i<N;i++){
        float val=1.0f/2.0f*(1.0f-cos(2*M_PI*(i+1)/(N+1)));
        hanning_window[i]=val;
    }
	for(int i=0;i<N/2;i++){
		freq[i]=1.0f*ADC_SAMPLE_RATE/N*i;
	}

    //xTaskCreate(tskFFT,"FFT",tskFFTStkSize,NULL,tskFFTPrio,&tskFFTHandle);
    xTaskCreate(tskFFTSimple,"FFT",tskFFTStkSize,NULL,tskFFTPrio,&tskFFTHandle);
}
