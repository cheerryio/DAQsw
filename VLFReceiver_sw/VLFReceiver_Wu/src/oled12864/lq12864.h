#ifndef __LQ12864_H__
#define __LQ12864_H__

#include <xil_types.h>
#include "../IicMaster/IicMaster.h"

class LQ12864
{
private:
	IicMaster *iic;
	u8 slvWr;
	u8 slvRd;
	static const u16 X_SIZE = 128;
	static const u16 Y_SIZE = 64;

	void writeData(u8 data);
	void writePage(u8 page, volatile u8 *data);
	void writeCmd(u8 cmd);
	void writeCmds(u8 *cmd, u8 len);
	void swap(int *x, int *y);

public:
	volatile u8 VBuf[Y_SIZE / 8][X_SIZE];
	LQ12864(IicMaster *iic, u8 slave7bit);
	~LQ12864();

	void FlushAll();
	void FlushPage(u8 page);
	void BufFill(bool c);
	void BufPixel(int x, int y, u8 c);
	void BufLine(int x0, int y0, int x1, int y1, u8 c);
	void BufBar(int x0, int y0, int x1, int y1, u8 lineColor, u8 fillColor);
	void BufBox(int x0,int y0,int x1,int y1,unsigned char c);
	int BufTextSmall(char *str, int x, int y, u8 fc, u8 bc);
	int VirtualTextSmall(char *str,int x);
};

/*********************OLED ��������************************************/
//void OLED_Set_Pos(unsigned char x, unsigned char y)
//{
//	OLED_WrCmd(0xb0+y);
//	OLED_WrCmd(((x&0xf0)>>4)|0x10);
//	OLED_WrCmd((x&0x0f)|0x01);
//}
///*********************OLEDȫ��************************************/
//void OLED_Fill(unsigned char bmp_dat)
//{
//	unsigned char y,x;
//	for(y=0;y<8;y++)
//	{
//		OLED_WrCmd(0xb0+y);
//		OLED_WrCmd(0x01);
//		OLED_WrCmd(0x10);
//		for(x=0;x<X_WIDTH;x++)
//		OLED_WrDat(bmp_dat);
//	}
//}
///*********************OLED��λ************************************/
//void OLED_CLS(void)
//{
//	unsigned char y,x;
//	for(y=0;y<8;y++)
//	{
//		OLED_WrCmd(0xb0+y);
//		OLED_WrCmd(0x01);
//		OLED_WrCmd(0x10);
//		for(x=0;x<X_WIDTH;x++)
//		OLED_WrDat(0);
//	}
//}
///*********************OLED��ʼ��************************************/
//void OLED_Init(void)
//{
//	delay(500);//��ʼ��֮ǰ����ʱ����Ҫ��
//	OLED_WrCmd(0xae);//--turn off oled panel
//	OLED_WrCmd(0x00);//---set low column address
//	OLED_WrCmd(0x10);//---set high column address
//	OLED_WrCmd(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
//	OLED_WrCmd(0x81);//--set contrast control register
//	OLED_WrCmd(Brightness); // Set SEG Output Current Brightness
//	OLED_WrCmd(0xa1);//--Set SEG/Column Mapping     0xa0���ҷ��� 0xa1����
//	OLED_WrCmd(0xc8);//Set COM/Row Scan Direction   0xc0���·��� 0xc8����
//	OLED_WrCmd(0xa6);//--set normal display
//	OLED_WrCmd(0xa8);//--set multiplex ratio(1 to 64)
//	OLED_WrCmd(0x3f);//--1/64 duty
//	OLED_WrCmd(0xd3);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
//	OLED_WrCmd(0x00);//-not offset
//	OLED_WrCmd(0xd5);//--set display clock divide ratio/oscillator frequency
//	OLED_WrCmd(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
//	OLED_WrCmd(0xd9);//--set pre-charge period
//	OLED_WrCmd(0xf1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
//	OLED_WrCmd(0xda);//--set com pins hardware configuration
//	OLED_WrCmd(0x12);
//	OLED_WrCmd(0xdb);//--set vcomh
//	OLED_WrCmd(0x40);//Set VCOM Deselect Level
//	OLED_WrCmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
//	OLED_WrCmd(0x02);//
//	OLED_WrCmd(0x8d);//--set Charge Pump enable/disable
//	OLED_WrCmd(0x14);//--set(0x10) disable
//	OLED_WrCmd(0xa4);// Disable Entire Display On (0xa4/0xa5)
//	OLED_WrCmd(0xa6);// Disable Inverse Display On (0xa6/a7)
//	OLED_WrCmd(0xaf);//--turn on oled panel
//	OLED_Fill(0x00); //��ʼ����
//	OLED_Set_Pos(0,0);
//}
///***************������������ʾ6*8һ���׼ASCII�ַ���	��ʾ�����꣨x,y����yΪҳ��Χ0��7****************/
//void OLED_P6x8Str(unsigned char x, y,unsigned char ch[])
//{
//	unsigned char c=0,i=0,j=0;
//	while (ch[j]!='\0')
//	{
//		c =ch[j]-32;
//		if(x>126){x=0;y++;}
//		OLED_Set_Pos(x,y);
//		for(i=0;i<6;i++)
//		OLED_WrDat(F6x8[c][i]);
//		x+=6;
//		j++;
//	}
//}
///*******************������������ʾ8*16һ���׼ASCII�ַ���	 ��ʾ�����꣨x,y����yΪҳ��Χ0��7****************/
//void OLED_P8x16Str(unsigned char x, y,unsigned char ch[])
//{
//	unsigned char c=0,i=0,j=0;
//	while (ch[j]!='\0')
//	{
//		c =ch[j]-32;
//		if(x>120){x=0;y++;}
//		OLED_Set_Pos(x,y);
//		for(i=0;i<8;i++)
//		OLED_WrDat(F8X16[c*16+i]);
//		OLED_Set_Pos(x,y+1);
//		for(i=0;i<8;i++)
//		OLED_WrDat(F8X16[c*16+i+8]);
//		x+=8;
//		j++;
//	}
//}
///*****************������������ʾ16*16����  ��ʾ�����꣨x,y����yΪҳ��Χ0��7****************************/
//void OLED_P16x16Ch(unsigned char x, y, N)
//{
//	unsigned char wm=0;
//	unsigned int adder=32*N;
//	OLED_Set_Pos(x , y);
//	for(wm = 0;wm < 16;wm++)
//	{
//		OLED_WrDat(F16x16[adder]);
//		adder += 1;
//	}
//	OLED_Set_Pos(x,y + 1);
//	for(wm = 0;wm < 16;wm++)
//	{
//		OLED_WrDat(F16x16[adder]);
//		adder += 1;
//	}
//}
///***********������������ʾ��ʾBMPͼƬ128��64��ʼ������(x,y),x�ķ�Χ0��127��yΪҳ�ķ�Χ0��7*****************/
//void Draw_BMP(unsigned char x0, y0,x1, y1,unsigned char BMP[])
//{
//	unsigned int j=0;
//	unsigned char x,y;
//
//  if(y1%8==0) y=y1/8;
//  else y=y1/8+1;
//	for(y=y0;y<y1;y++)
//	{
//		OLED_Set_Pos(x0,y);
//    for(x=x0;x<x1;x++)
//	    {
//	    	OLED_WrDat(BMP[j++]);
//	    }
//	}
//}
#endif
