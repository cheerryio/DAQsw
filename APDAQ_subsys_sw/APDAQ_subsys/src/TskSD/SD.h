#ifndef __SD_H__
#define __SD_H__

#include "../common.h"


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
