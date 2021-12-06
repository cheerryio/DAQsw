#ifndef __COMMON_H__
#define __COMMON_H__

#include "xparameters.h"
#include "XilPeriph/XilAxiDma.h"
#include "XilPeriph/Eth.h"
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

#define DMA_TRANS_SIZE 16000        ///< dma单次传输数据数量
#define DA_SAMPLE_RATE 2e6          ///< da输出采样率

#define IP_ADDR "192.168.1.10"      ///< FPGA开发板 服务器ip地址
#define NETMASK "255.255.255.0"     ///< FPGA开发板 服务器掩码
#define GW      "192.168.1.1"       ///< FPGA开发板 服务器网关
#define SERVER_PORT 7               ///< FPGA开发板 服务器端口

#define DATA_BUF_NUM 2                ///< netbuf多级缓冲数量
#define DATA_BUF_SIZE 2000000         ///< netbuf大小

#define DEBUG 0

void initPeriph();

#endif
