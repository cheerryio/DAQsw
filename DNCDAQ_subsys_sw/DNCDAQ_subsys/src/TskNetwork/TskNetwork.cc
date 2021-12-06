#include "TskNetwork.h"
#include <xparameters.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "xil_io.h"
#include "../common.h"

using namespace std;

extern XilGpio* gpio_adc_ctrl;
extern Eth* net;

const int tskNetworkPrio = 2;
const int tskNetworkStkSize = 1024;
static TaskHandle_t tskNetworkHandle;

extern __attribute__((aligned(32))) int databuf[DATA_BUF_NUM][DATA_BUF_SIZE];
extern SemaphoreHandle_t semDataBufWr;
extern SemaphoreHandle_t semDataBufRd;
unsigned int databuf_rd_p=0;

extern SemaphoreHandle_t semDataReady;

static void tskNetworkSub(void *pvParameters){
	int sock=*(int*)pvParameters;
	int bytes_send;
    int* p;

    while(1){
        // connect to server and get socket
        unsigned int cnt;
        while(1){
            gpio_adc_ctrl->XilGpioDiscreteWrite(1,0);
            vTaskDelay(500);
            gpio_adc_ctrl->XilGpioDiscreteWrite(1,1);
            vTaskDelay(500);
            cnt=gpio_adc_ctrl->XilGpioDiscreteRead(2);
            if(cnt>0){
                break;
            }
            else{
                printf("restart,cnt=%d\n\r",cnt);
            }
        }
        xSemaphoreGive(semDataReady);

        // get data from buffers and send through networks
        while(1){
            if(xSemaphoreTake(semDataBufRd,portMAX_DELAY)){
                p=databuf[databuf_rd_p];
				bytes_send=net->send_packet(sock,(void*)(p),DATA_BUF_SIZE*sizeof(int),MSG_MORE);
				if(bytes_send<0){
					xSemaphoreGive(semDataBufRd);
					close(sock);
					break;
				}else{
                    databuf_rd_p=databuf_rd_p==DATA_BUF_NUM-1?0:databuf_rd_p+1;
					xSemaphoreGive(semDataBufWr);
				}
            }
        }
    }

    vTaskDelete(NULL);
}

static void tskNetwork(void* pvParameters){
	printf("in network task\n\r");
	int sock;
    // initialize lwip basic settings
	net->init();
	net->mac_phy_auto_negotiation();
	sys_thread_new("xemacif_input_thread",(void(*)(void*))xemacif_input_thread, &net->net_if,1024,DEFAULT_THREAD_PRIO);
    net->assign_ip();
    sock=net->connect_server(SERVER_IP_ADDR,SERVER_PORT);
    if(sock<0){
        printf("create sock error\n\r");
    }else{
    	printf("create sock success\n\r");
        xTaskCreate(tskNetworkSub,"tskNetworkSub",configMINIMAL_STACK_SIZE,&sock,2,NULL);
    }

    vTaskDelete(NULL);
}

void tskNetworkInit()
{
    tskNetworkHandle=sys_thread_new("tskNetwork",tskNetwork,NULL,tskNetworkStkSize,tskNetworkPrio);
    configASSERT(tskNetworkHandle);
}
