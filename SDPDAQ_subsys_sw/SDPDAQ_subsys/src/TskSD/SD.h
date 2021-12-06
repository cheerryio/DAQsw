#ifndef _SD_H_
#define _SD_H_

#include "../common.h"

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
		SampleRate = 450000;
		ByteRate = 450000 * 3;
		BlockAlign = 3;
		BitsPerSample = 24;
		Subchunk2ID[0] = 'd'; Subchunk2ID[1] = 'a'; Subchunk2ID[2] = 't'; Subchunk2ID[3] = 'a';
		Subchunk2Size = 0;// + 92160000; //FILE_BYTES_PER_SMP * NCH * SRATE_DECI * 60 * MINUTES_PER_FILE;
	}
};


int SDinit();
int SDRead(int *data, unsigned int len, char *fileName);
int SDWriteInt(int *data, unsigned int len, char *fileName);
int SDWriteFloat(float *data, unsigned int len, char *fileName);
int SDWriteDouble(double *data, unsigned int len, char *fileName);
int SDWriteString(char *data, unsigned int len, char *fileName);
int SDWriteWavHeader(char *fileName);
int openDir(char *dirName);
int closeDir();
int openFile(char *fileName);
int createFile(char *fileName);
int closeFile(char *fileName);
int nativeWrite(char *data, unsigned int len);
int fileSeek(unsigned int pos);
int fileSync();
int seekFileEnd();
int getSDFreeCapacity(void);
int SDTest();
#endif
