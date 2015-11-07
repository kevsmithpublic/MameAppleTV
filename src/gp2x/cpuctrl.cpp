/*  cpuctrl.c for GP2X (CPU/LCD/RAM-Tuner Version 2.0)
    Copyright (C) 2006 god_at_hell 
    original CPU-Overclocker (c) by Hermes/PS2Reality 
	the gamma-routine was provided by theoddbot
	parts (c) Rlyehs Work

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/


#include <sys/mman.h>
#include "cpuctrl.h"
#include <stdio.h>
#include <math.h>

/* system registers */
static struct 
{
	unsigned short SYSCLKENREG,SYSCSETREG,FPLLVSETREG,DUALINT920,DUALINT940,DUALCTRL940,DISPCSETREG;
}
system_reg;

static volatile unsigned short *MEM_REG;

#define SYS_CLK_FREQ 7372800


void cpuctrl_init(void)
{
	extern volatile unsigned short *gp2x_memregs; /* from minimal library rlyeh */
	MEM_REG=&gp2x_memregs[0];
	system_reg.SYSCSETREG=MEM_REG[0x91c>>1];
	system_reg.FPLLVSETREG=MEM_REG[0x912>>1];
	system_reg.SYSCLKENREG=MEM_REG[0x904>>1];
	system_reg.DUALINT920=MEM_REG[0x3B40>>1];
	system_reg.DUALINT940=MEM_REG[0x3B42>>1];
	system_reg.DUALCTRL940=MEM_REG[0x3B48>>1];
	system_reg.DISPCSETREG=MEM_REG[0x924>>1];
}


void cpuctrl_deinit(void)
{
	MEM_REG[0x91c>>1]=system_reg.SYSCSETREG;
	MEM_REG[0x910>>1]=system_reg.FPLLVSETREG;
	MEM_REG[0x3B40>>1]=system_reg.DUALINT920;
	MEM_REG[0x3B42>>1]=system_reg.DUALINT940;
	MEM_REG[0x3B48>>1]=system_reg.DUALCTRL940;
	MEM_REG[0x904>>1]=system_reg.SYSCLKENREG;
	MEM_REG[0x924>>1]=system_reg.DISPCSETREG;
}


void set_display_clock_div(unsigned divider)
{
	divider=((divider & 63) | 64)<<8;
	MEM_REG[0x924>>1]=(MEM_REG[0x924>>1] & ~(255<<8)) | divider;
}


void set_FCLK(unsigned MHZ)
{
	unsigned v;
	unsigned mdiv,pdiv=3,scale=0;
	MHZ*=1000000;
	mdiv=(MHZ*pdiv)/SYS_CLK_FREQ;
	mdiv=((mdiv-8)<<8) & 0xff00;
	pdiv=((pdiv-2)<<2) & 0xfc;
	scale&=3;
	v=mdiv | pdiv | scale;
	MEM_REG[0x910>>1]=v;
}


void set_920_Div(unsigned short divider)
{
	unsigned short v;
	v = MEM_REG[0x91c>>1] & (~0x3);
	MEM_REG[0x91c>>1] = (divider & 0x7) | v; 
}


void set_DCLK_Div( unsigned short divider )
{
	unsigned short v;
	v = (unsigned short)( MEM_REG[0x91c>>1] & (~(0x7 << 6)) );
	MEM_REG[0x91c>>1] = ((divider & 0x7) << 6) | v; 
}


void Disable_940(void)
{
	MEM_REG[0x3B42>>1];
	MEM_REG[0x3B42>>1]=0;
	MEM_REG[0x3B46>>1]=0xffff;	
	MEM_REG[0x3B48>>1]|= (1 << 7);
	MEM_REG[0x904>>1]&=0xfffe;
}

void gp2x_video_wait_vsync(void)
{
	static int settings=0;

	if (!settings)
	{
		if(MEM_REG[0x2800>>1]&0x100)
		{
			/* TV-Out */
		}
		else
		{
			/* No TV-Out */
			/* Change LCD Refresh Values to make VSync to run correctly */
			typedef struct { unsigned short reg, valmask, val; } reg_setting;
			// 120.00 97/0/2/7|25/ 7/ 7/11/37
			static reg_setting rate_120[] =
			{
			        { 0x0914, 0xffff, (97<<8)|(0<<2)|2 },   /* UPLLSETVREG */
			        { 0x0924, 0xff00, (2<<14)|(7<<8) },     /* DISPCSETREG */
			        { 0x281A, 0x00ff, 25 },                 /* .HSWID(T2) */
			        { 0x281C, 0x00ff, 7 },                  /* .HSSTR(T8) */
			        { 0x281E, 0x00ff, 7 },                  /* .HSEND(T7) */
			        { 0x2822, 0x01ff, 11 },                 /* .VSEND (T9) */
			        { 0x2826, 0x0ff0, 37<<4 },              /* .DESTR(T3) */
			        { 0, 0, 0 }
			};
			reg_setting *set= rate_120;
			for (; set->reg; set++)
			{
				unsigned short val = MEM_REG[set->reg >> 1];
				val &= ~set->valmask;
				val |= set->val;
				MEM_REG[set->reg >> 1] = val;
			}
		}
		settings=1;
	}

	unsigned short v = MEM_REG[0x1182>>1];
	while (!((v ^ MEM_REG[0x1182>>1]) & 0x10));
}

void set_tRC(unsigned short timing)
{
	//printf ("set tRC = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3804>>1] & (~(0xF << 8)));
	MEM_REG[0x3804>>1] = ((timing & 0xF) << 8) | v;	
}

void set_tRAS(unsigned short timing)
{
	//printf ("set tRAS = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3804>>1] & (~(0xF << 4)));
	MEM_REG[0x3804>>1] = ((timing & 0xF) << 4) | v;	
}

void set_tWR(unsigned short timing)
{	
	//printf ("set tWR = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3804>>1] & (~(0xF)));
	MEM_REG[0x3804>>1] = (timing & 0xF) | v;	
}

void set_tMRD(unsigned short timing)
{
	//printf ("set tMRD = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3802>>1] & (~(0xF << 12)));
	MEM_REG[0x3802>>1] = ((timing & 0xF) << 12) | v;	
}

void set_tRFC(unsigned short timing)
{
	//printf ("set tRFC = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3802>>1] & (~(0xF << 8)));
	MEM_REG[0x3802>>1] = ((timing & 0xF) << 8) | v;	
}

void set_tRP(unsigned short timing)
{
	//printf ("set tRP = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3802>>1] & (~(0xF << 4)));
	MEM_REG[0x3802>>1] = ((timing & 0xF) << 4) | v;	
}

void set_tRCD(unsigned short timing)
{
	//printf ("set tRCD = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3802>>1] & (~(0xF)));
	MEM_REG[0x3802>>1] = (timing & 0xF) | v;	
}

void set_gamma(int g100)
{
	float gamma = (float) g100 / 100;
	int i;
	gamma = 1/gamma;
    	//enable gamma
    	MEM_REG[0x2880>>1]&=~(1<<12);
    	MEM_REG[0x295C>>1]=0;
    	for(i=0; i<256; i++)
    	{
		unsigned char g;
        	unsigned short s;
        	g =(unsigned char)(255.0*pow(i/255.0,gamma));
        	s = (g<<8) | g;
		MEM_REG[0x295E>>1]= s;
        	MEM_REG[0x295E>>1]= g;
    	}
}
