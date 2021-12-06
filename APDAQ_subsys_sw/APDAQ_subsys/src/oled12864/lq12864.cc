#include "lq12864.h"
#include "oled_fonts.h"
#include "../IicMaster/IicMaster.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SIGN(x) ((x) < 0 ? -1 : 1)

LQ12864::LQ12864(IicMaster *iic, u8 slave7bit)
{
	this->iic = iic;
	this->slvWr = (slave7bit << 1) | 0x0;
	this->slvRd = (slave7bit << 1) | 0x1;

	BufFill(false);

	u8 initCmd[28] = {
        0xae,   // turn off oled panel
        0x00,   // set low column address
        0x10,   // set high column address
        0x40,   // set start line address
        0x81,   // set contrast control register
        0xc0,   // set seg output current brightness
        0xa0,   // set seg/column mapping
        0xc0,   // set com/row scan direction
        0xa6,   // set normal display
        0xa8,   // set multiplex ratio(1 to 64)
        0x3f,   // 64
        0xd3,   // set display offset
        0x00,   // no offset
        0xd5,   // set display clock divide ratio/oscillator frequency
        0x80,   // set divide ratio, to 100 fps
        0xd9,   // set pre-charge period
        0xf1,   // set pre-charge as 15 clocks & discharge as 1 clock
        0xda,   // set com pins hardware configuration
        0x12,   
        0xdb,   // set vcomh
        0x40,   // set vcom deselect level
        0x20,   // set page addressing mode (0x00/0x01/0x02)
        0x02,   //
        0x8d,   // set charge pump enable/disable
        0x14,   // set(0x10) disable
        0xa4,   // disable entire display on (0xa4/0xa5)
        0xa6,   // disable inverse display on (0xa6/a7)
        0xaf    // turn on oled panel
    };
	writeCmds(initCmd, 28);
	u8 t0, t1;
	while(iic->CmdLen() > 0)
	{
		while(iic->DataLen() > 0)
			iic->PopData(&t0, &t1);
	}
//	onlineSta |= 0x01;
}

LQ12864::~LQ12864(){}

void LQ12864::writeData(u8 data)
{
	iic->PushCmd(IicMaster::Start, 0);
	// select slave for wr
	iic->PushCmd(IicMaster::WriteByte, slvWr);
	// control : data
	iic->PushCmd(IicMaster::WriteByte, 0x40);
	// data
	iic->PushCmd(IicMaster::WriteByte, data);
	iic->PushCmd(IicMaster::Stop, 0);
}

void LQ12864::writePage(u8 page, volatile u8 *data)
{
	u8 cmds[3] = {(0xb0 | (page & 0x7)), 0x01, 0x10};
	writeCmds(cmds, 3);
	iic->PushCmd(IicMaster::Start, 0);
	// select slave for wr
	iic->PushCmd(IicMaster::WriteByte, slvWr);
	// control : data
	iic->PushCmd(IicMaster::WriteByte, 0x40);
	// data
	for(int i = 0; i < this->X_SIZE; i++)
	{
		iic->PushCmd(IicMaster::WriteByte, data[i]);
	}
	iic->PushCmd(IicMaster::Stop, 0);
}

void LQ12864::writeCmd(u8 cmd)
{
	iic->PushCmd(IicMaster::Start, 0);
	// select slave for wr
	iic->PushCmd(IicMaster::WriteByte, slvWr);
	// control : single cmd
	iic->PushCmd(IicMaster::WriteByte, 0x00);
	// cmd
	iic->PushCmd(IicMaster::WriteByte, cmd);
	iic->PushCmd(IicMaster::Stop, 0);
}

void LQ12864::writeCmds(u8 *cmd, u8 len)
{
	iic->PushCmd(IicMaster::Start, 0);
	// select slave for wr
	iic->PushCmd(IicMaster::WriteByte, slvWr);
	// control : cmd
	iic->PushCmd(IicMaster::WriteByte, 0x00);
	// cmd
	for(int i = 0; i < len ; i++)
	{
		iic->PushCmd(IicMaster::WriteByte, cmd[i]);
	}
	iic->PushCmd(IicMaster::Stop, 0);
}

void LQ12864::detect()
{
//	u8 sta =iic->WriteRegPop(1);
//
//	if(sta & 0x01){	// iic device onlice
//		if(onlineSta & 0x10)	// last sta online
//			onlineSta |= 0x10;	// keep sta
//		else
//			onlineSta |= 0x20;
//	}
//	else{			// iic device offline
//		if(onlineSta & 0x01)	// last sta online
//			onlineSta |= 0x20;	//
//		else
//			onlineSta |= 0x80;
//	}
//	if(onlineSta & 0x80)
//	{
//
//	}

}
void LQ12864::FlushAll()
{
	for(int p = 0; p < this->Y_SIZE / 8; p++)
	{
		FlushPage(p);
	}
}

void LQ12864::FlushPage(u8 page)
{
	writePage(page, &(this->VBuf[page][0]));
}

void LQ12864::BufFill(bool c)
{
	for(int p = 0; p < this->Y_SIZE / 8; p++)
	{
		for(int x = 0; x < this->X_SIZE; x++)
		{
			this->VBuf[p][x] = c? 0xFF : 0x00;
		}
	}
}

void LQ12864::BufPixel(int x, int y, u8 c)
{
	u8 tmp;
	x &= 0x7F;
	y &= 0x3F;
	tmp = this->VBuf[y >> 3][x];
	if(c == 0)
	{
		tmp &= ~(1 << (y & 0x7));
	}
	else if(c == 1)
	{
		tmp |= (1 << (y & 0x7));
	}
	else
	{
		tmp ^= (1 << (y & 0x7));
	}
	this->VBuf[y >> 3][x] = tmp;
}

inline void LQ12864::swap(int *x, int *y)
{
	*x += *y;
	*y = *x - *y;
	*x -= *y;
}

void LQ12864::BufLine(int x0, int y0, int x1, int y1, u8 c)
{
	int x, y, sx, sy, e, dx, dy, temp;
	dx = x1 - x0;
	dy = y1 - y0;
	if(!(dx | dy))
		return;
	if(ABS(dx) > ABS(dy))
	{
		if(x0 > x1)
		{
			temp = x0;
			x0 = x1;
			x1 = temp;
			temp = y0;
			y0 = y1;
			y1 = temp;
		}
		dx = x1 - x0;
		dy = y1 - y0;
		sy = SIGN(dy);
		dy = ABS(dy);
		e = (dy >> 1);
		y = y0;
		for(x = x0; x <= x1; x++)
		{
			BufPixel(x, y, c);
			e += dy;
			if(e >= dx)
			{
				y += sy;
				e -= dx;
			}
		}
	}
	else
	{
		if(y0 > y1)
		{
			temp = x0;
			x0 = x1;
			x1 = temp;
			temp = y0;
			y0 = y1;
			y1 = temp;
		}
		dx = x1 - x0;
		dy = y1 - y0;
		sx = SIGN(dx);
		dx = ABS(dx);
		e = (dx >> 1);
		x = x0;
		for(y = y0; y <= y1; y++)
		{
			BufPixel(x, y, c);
			e += dx;
			if(e >= dy)
			{
				x += sx;
				e -= dy;
			}
		}
	}
}

void LQ12864::BufBar(int x0, int y0, int x1, int y1,
		u8 lineColor, u8 fillColor)
{
	int x, y, temp;
	if(x0 > x1)
	{
		temp = x0;
		x0 = x1;
		x1 = temp;
	}
	for(x = x0 + 1; x < x1; x++)
	{
//		*P_Watchdog_Clear=1;
		for(y = y0 + 1; y < y1; y++)
			BufPixel(x, y, fillColor);
	}
	for(x=x0;x<=x1;x++)
	{
		BufPixel(x, y0, lineColor);
		BufPixel(x, y1, lineColor);
	}
	for(y=y0+1;y<y1;y++)
	{
		BufPixel(x0, y, lineColor);
		BufPixel(x1, y, lineColor);
	}
}

void LQ12864::BufBox(int x0,int y0,int x1,int y1,unsigned char c)
{
	int x, y, temp;
	if(x0 > x1)
	{
		temp = x0;
		x0 = x1;
		x1 = temp;
	}
	for(x = x0; x <= x1; x++)
	{
		BufPixel(x, y0, c);
		BufPixel(x, y1, c);
	}
	for(y = y0 + 1; y < y1; y++)
	{
		BufPixel(x0, y, c);
		BufPixel(x1, y, c);
	}
}

int LQ12864::BufTextSmall(char *str, int x, int y, u8 fc, u8 bc)	//Color:0-white,1-black,2-reverse,3-lucency
{
	int i, j;
#ifdef __CN_CHAR__
	int k, n, qu, wei, location;
#endif
	while(*str)
	{
		if(*str < 128)
		{
			for(i = 0; i < 6; i++)
			{
				for(j = 0; j < 8; j++)
				{
					if((0x1 << j) & Font6x8[*str - 32][i])
					{
						if(fc != 3)
						{
							BufPixel(x + i, y + j, fc);
						}
					}
					else
					{
						if(bc != 3)
						{
							BufPixel(x + i, y + j, bc);
						}
					}
				}
			}
			x+=6;
			str++;
		}
#ifdef __CN_CHAR__
		else
		{
			qu=*(str++)-0xa0;
			wei=*(str++)-0xa0;
			location=qu*94+wei;
			for(n=0;n<NUM_STR;n++)
			{
				if(ZKQW[n]==location)
				{
					for(i=0;i<12;i++)
					{
							for(k=0;k<8;k++)
								if((ZKDATA[n*24+i*2]>>(7-k))&0x01)
								{
									if(ForeColor!=3)
										LCD_Pixel(x+k,y+i,ForeColor);
								}
								else
								{
									if(ForeColor!=3)
										LCD_Pixel(x+k,y+i,BackColor);
								}
							for(k=0;k<4;k++)
								if((ZKDATA[n*24+i*2+1]>>(7-k))&0x01)
								{
									if(ForeColor!=3)
										LCD_Pixel(x+8+k,y+i,ForeColor);
								}
								else
								{
									if(ForeColor!=3)
										LCD_Pixel(x+8+k,y+i,BackColor);
								}
					}
					break;
				}
			}
			x+=12;
		}
#endif
	}
	return x;
}

int LQ12864::VirtualTextSmall(char *str,int x)	//for getting the pixel length of str
{
	while(*str)
	{
		if(*str<128)
		{
			x+=6;
			str++;
		}
#ifdef __CN_CHAR__
		else
		{
			str+=2;
			x+=12;
		}
#endif
	}
	return x;
}
