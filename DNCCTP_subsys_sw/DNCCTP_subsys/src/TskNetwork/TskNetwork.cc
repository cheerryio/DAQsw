#include "TskNetwork.h"
#include <xparameters.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "xil_io.h"
#include "../common.h"

using namespace std;

extern Eth* net;

const int tskNetworkPrio = 2;
const int tskNetworkStkSize = 16384;
static TaskHandle_t tskNetworkHandle;

const int tskNetworkSubPrio = 2;
const int tskNetworkSubStkSize = 16384;
static TaskHandle_t tskNetworkSubHandle;

__attribute__((aligned(32))) u16 databuf[DATA_BUF_NUM][DATA_BUF_SIZE];
SemaphoreHandle_t semDataBufWr;
SemaphoreHandle_t semDataBufRd;
unsigned int databuf_wr_p=0;

static void tskNetworkSub(void *pvParameters){
    u16* p;
    int readbytes;
    int readbytes_total;
    int bytes_send;
    int sock=*((int *)pvParameters);
    char* msg="send me data";

    printf("start new collect traffic thread\n\r");
    while(xSemaphoreTake(semDataBufWr,portMAX_DELAY)){
        readbytes_total=0;
        p=databuf[databuf_wr_p];
        
        // tell matlab to send me data
        bytes_send=lwip_send(sock,msg,strlen(msg),0);
        while(1){
            readbytes=lwip_recvfrom(sock,((void*)p+readbytes_total),DATA_BUF_SIZE*sizeof(u16)-readbytes_total,0,NULL,NULL);
            if(readbytes<0){
                printf("tcp abort remote\n\r");
                vTaskDelay(1000);
                continue;
            }
            else if(readbytes==0){
                printf("read bytes is 0\n\r");
                continue;
            }
            else{
                // receive 2000000 data
                readbytes_total=readbytes_total+readbytes;
                if(readbytes_total>=DATA_BUF_SIZE*sizeof(u16)){
                    databuf_wr_p=databuf_wr_p==DATA_BUF_NUM-1?0:databuf_wr_p+1;
                    if(DEBUG)
                        printf("giving semDataBufRd\n\r");
                    xSemaphoreGive(semDataBufRd);
                    break;
                }
            }
        }
    }

    vTaskDelete(NULL);
}

static void tskNetwork(void* pvParameters){
    int sock,new_sock;
    struct sockaddr_in remote;
    int remote_size;
    
    // initialize lwip basic settings
	net->init();
	net->mac_phy_auto_negotiation();
	sys_thread_new("xemacif_input_thread",(void(*)(void*))xemacif_input_thread, &net->net_if,1024,DEFAULT_THREAD_PRIO);
    net->assign_ip();
    sock=net->launch_server(SERVER_PORT);
    if(sock<0){
		printf("error create sock\n\r");
		vTaskDelete(NULL);
	}
    configASSERT(sock>=0);
    remote_size=sizeof(remote);
    while(1){
        if((new_sock=accept(sock,(struct sockaddr*)&remote,(socklen_t*)&remote_size))>0){
            if(tskNetworkSubHandle!=NULL){
                vTaskDelete(tskNetworkSubHandle);
            }
            for(int i=0;i<DATA_BUF_NUM;i++){
                xSemaphoreGive(semDataBufWr);
            }
            tskNetworkSubHandle=sys_thread_new("tskNetworkSub",tskNetworkSub,(void*)&new_sock,4096,2);
        }
    }

    vTaskDelete(NULL);
}

void tskNetworkInit()
{
    semDataBufWr=xSemaphoreCreateCounting(DATA_BUF_NUM,DATA_BUF_NUM);
    configASSERT(semDataBufWr);

    semDataBufRd=xSemaphoreCreateCounting(DATA_BUF_NUM,0);
    configASSERT(semDataBufRd);

    tskNetworkHandle=sys_thread_new("tskNetwork",tskNetwork,NULL,1024,tskNetworkPrio);
    configASSERT(tskNetworkHandle);
}
