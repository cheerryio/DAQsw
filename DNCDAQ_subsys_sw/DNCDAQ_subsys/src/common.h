#ifndef __COMMON_H__
#define __COMMON_H__

#include "xparameters.h"
#include "XilPeriph/XilGpio.h"
#include "XilPeriph/Eth.h"
#include "XilPeriph/XilAxiDma.h"
#include "xscugic.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

/**
 * @file  common.h
 * @brief 项目全局宏定义配置
 * @details 配置网络信息，buffer大小数量信息，dma传输数量信息
 * @author Duna
 * @version 1.0.0
 */

#define CLIENT_IP_ADDR "192.168.1.30"       ///< 配置FPGA开发板ip地址
#define CLIENT_NETMASK "255.255.255.0"      ///< 配置FPGA开发板掩码
#define CLIENT_GW      "192.168.1.1"        ///< 配置FPGA开发板网关

#define SERVER_IP_ADDR "192.168.1.42"       ///< 配置上位机ip地址
#define SERVER_PORT    7                    ///< 配置上位机端口

#define DMA_TRANS_SIZE 10000                ///< dma一次搬运数据数量

#define DATA_BUF_SIZE 2000000                 ///< databuf大小，代表一次lwip_send发送数据数量
#define DATA_BUF_NUM 8                        ///< databuf多级缓冲数量

#define DEBUG 0

void initPeriph();

#endif
