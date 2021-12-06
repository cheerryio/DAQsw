#ifndef __COMMON_H__
#define __COMMON_H__

#include "xparameters.h"
#include "XilPeriph/XilGpio.h"
#include "XilPeriph/XilAxiDma.h"
#include <FreeRTOS.h>
#include <semphr.h>
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

#define SAMPLE_RATE 512000
#define SECONDS_PER_FILE 60
#define DEBUG 1

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
	unsigned int ByteRate;// 512k * 3 *1ch;
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
		SampleRate = 512000;
		ByteRate = 512000 * 3;
		BlockAlign = 3;
		BitsPerSample = 24;
		Subchunk2ID[0] = 'd'; Subchunk2ID[1] = 'a'; Subchunk2ID[2] = 't'; Subchunk2ID[3] = 'a';
		Subchunk2Size = 0;// + 92160000; //FILE_BYTES_PER_SMP * NCH * SRATE_DECI * 60 * MINUTES_PER_FILE;
	}
};
void initPeriph();

#endif
