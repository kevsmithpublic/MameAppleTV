#ifndef __POLLUXREGS_H__
#define __POLLUXREGS_H__

/* CPU control */

#define PLLSETREG0		(memregs32[0xF004>>2])
#define PWRMODE			(memregs32[0xF07C>>2])

/* MLC */
#define	MLCCONTROLT		(memregs32[0x4000>>2])
#define	MLCSCREENSIZE		(memregs32[0x4004>>2])
#define	MLCBGCOLOR		(memregs32[0x4008>>2])

#define	MLCLEFTRIGHT0		(memregs32[0x400C>>2])
#define	MLCTOPBOTTOM0		(memregs32[0x4010>>2])
#define MLCLEFTRIGHT0_0		(memregs32[0x4014>>2])
#define	MLCTOPBOTTOM0_0		(memregs32[0x4018>>2])
#define MLCLEFTRIGHT0_1		(memregs32[0x401C>>2])
#define	MLCTOPBOTTOM0_1		(memregs32[0x4020>>2])
#define	MLCCONTROL0		(memregs32[0x4024>>2])
#define	MLCHSTRIDE0		(memregs32[0x4028>>2])
#define	MLCVSTRIDE0		(memregs32[0x402C>>2])
#define	MLCTPCOLOR0		(memregs32[0x4030>>2])
#define	MLCINVCOLOR0		(memregs32[0x4034>>2])
#define	MLCADDRESS0		(memregs32[0x4038>>2])
#define	MLCPALETTE0		(memregs32[0x403C>>2])

#define	MLCLEFTRIGHT1		(memregs32[0x4040>>2])
#define	MLCTOPBOTTOM1		(memregs32[0x4044>>2])
#define MLCLEFTRIGHT1_0		(memregs32[0x4048>>2])
#define	MLCTOPBOTTOM1_0		(memregs32[0x404C>>2])
#define MLCLEFTRIGHT1_1		(memregs32[0x4050>>2])
#define	MLCTOPBOTTOM1_1		(memregs32[0x4054>>2])
#define	MLCCONTROL1		(memregs32[0x4058>>2])
#define	MLCHSTRIDE1		(memregs32[0x405C>>2])
#define	MLCVSTRIDE1		(memregs32[0x4060>>2])
#define	MLCTPCOLOR1		(memregs32[0x4064>>2])
#define	MLCINVCOLOR1		(memregs32[0x4068>>2])
#define	MLCADDRESS1		(memregs32[0x406C>>2])
#define	MLCPALETTE1		(memregs32[0x4070>>2])

/* Graphics modes */
enum {
	RGB565		= 0x4432,
	BGR565		= 0xC432,
	XRGB1555	= 0x4342,
	XBGR1555	= 0xC342,
	XRGB4444	= 0x4211,
	XBGR4444	= 0xC211,
	XRGB8332	= 0x4120,
	XBGR8332	= 0xC120,
	ARGB1555	= 0x3342,
	ABGR1555	= 0xB342,
	ARGB4444	= 0x2211,
	ABGR4444	= 0xA211,
	ARGB8332	= 0x1120,
	ABGR8332	= 0x9120,
	RGB888		= 0x4653,
	BGR888		= 0xC653,
	XRGB8888	= 0x4653,
	XBGR8888	= 0xC653,
	ARGB8888	= 0x0653,
	ABGR8888	= 0x8653,
	PTRGB565	= 0x443A
};

/* Display controller */
#define DPCHTOTAL		(memregs16[0x307C>>1])
#define DPCHSWIDTH		(memregs16[0x307E>>1])
#define DPCHASTART		(memregs16[0x3080>>1])
#define DPCHAEND		(memregs16[0x3082>>1])
#define DPCVTOTAL		(memregs16[0x3084>>1])
#define DPCVSWIDTH		(memregs16[0x3086>>1])
#define DPCVASTART		(memregs16[0x3088>>1])
#define DPCVAEND		(memregs16[0x308A>>1])
#define DPCCTRL0		(memregs16[0x308C>>1])
#define DPCCTRL1		(memregs16[0x308E>>1])
#define DPCCLKGEN0		(memregs16[0x31C4>>1])
#define DITHER_NONE	0
#define DITHER_4BIT	1
#define DITHER_5BIT	2
#define DITHER_6BIT	3

/* GPIO */
#define GPIOAPAD		(memregs16[0xA018>>1])
#define GPIOBPAD		(memregs16[0xA058>>1])
#define GPIOCPAD		(memregs16[0xA098>>1])

/* RTC */
#define RTCCNTWRITE		(memregs32[0xF080>>2])
#define RTCCNTREAD		(memregs32[0xF084>>2])
#define RTCCTRL			(memregs32[0xF08C>>2])

/* TIMER */
#define TIMER_BASE3 0x1980
#define TIMER_REG(x) (memregs32[(TIMER_BASE3 + x) >> 2])

#endif
