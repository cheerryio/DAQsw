#ifndef _COMMON_H_
#define _COMMON_H_

#include "xparameters.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "xscugic.h"

#define DEBUG 0
#define CHANNEL_NUM 4
#define DATABUF_NUM 60
#define SD_BUFFERED_NUM 10
#define DMA_TRANS_SIZE 4000                ///< dma一次搬运数据数量
#define SAMPLE_RATE 256000
#define REALIGNED_SAMPLE_RATE 192000
#define SECONDS_PER_FILE 60

#define IIC_MASTER_BASEADDR 0x40001000
#define IIC_MASTER_IRQ 0U

struct MonoWavHeader
{
	char ChunkID[4];// = {'R','I','F','F'};
	unsigned int ChunkSize;// = 36 + data bytes;
	char Format[4];// = {'W','A','V','E'};
	char Subchunk1ID[4];// = {'f','m','t',' '};
	unsigned int Subchunk1Size;// = 16;
	unsigned short AudioFormat;// = 1;	// PCM
	unsigned short NumChannels;//;
	unsigned int SampleRate;//;
	unsigned int ByteRate;// 256k * 3 *1ch;
	unsigned short BlockAlign;// 3(24b) * 1ch;
	unsigned short BitsPerSample;// = 24;
	char Subchunk2ID[4];// = {'d','a','t','a'};
	unsigned int Subchunk2Size;// = FILE_BYTES_PER_SMP * NCH * SRATE_DECI * 60 * MINUTES_PER_FILE;
	MonoWavHeader()
	{
		ChunkID[0] = 'R'; ChunkID[1] = 'I'; ChunkID[2] = 'F'; ChunkID[3] = 'F';
		ChunkSize = 36;// + 92160000; // + FILE_BYTES_PER_SMP * NCH * SRATE_DECI * 60 * MINUTES_PER_FILE;
		Format[0] = 'W'; Format[1] = 'A'; Format[2] = 'V'; Format[3] = 'E';
		Subchunk1ID[0] = 'f'; Subchunk1ID[1] = 'm'; Subchunk1ID[2] = 't'; Subchunk1ID[3] = ' ';
		Subchunk1Size = 16;
		AudioFormat = 1;
		NumChannels = 1;
		SampleRate = 256000;
		ByteRate = 256000 * 3;
		BlockAlign = 3;
		BitsPerSample = 24;
		Subchunk2ID[0] = 'd'; Subchunk2ID[1] = 'a'; Subchunk2ID[2] = 't'; Subchunk2ID[3] = 'a';
		Subchunk2Size = 0;// + 92160000; //FILE_BYTES_PER_SMP * NCH * SRATE_DECI * 60 * MINUTES_PER_FILE;
	}
};

void initPeriph();

#endif
