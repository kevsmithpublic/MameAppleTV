/*

  GP2X minimal library v0.A by rlyeh, (c) 2005. emulnation.info@rlyeh (swap it!)

  Thanks to Squidge, Robster, snaff, Reesy and NK, for the help & previous work! :-)

  License
  =======

  Free for non-commercial projects (it would be nice receiving a mail from you).
  Other cases, ask me first.

  GamePark Holdings is not allowed to use this library and/or use parts from it.

*/

#include <fcntl.h>
#include <linux/fb.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include "uppermem.h"
#include "cpuctrl.h"
#include "usbjoy_mame.h"

#ifndef __MINIMAL_H__
#define __MINIMAL_H__

#define gp2x_video_color8(C,R,G,B) (gp2x_palette[((C)<<1)+0]=((G)<<8)|(B),gp2x_palette[((C)<<1)+1]=(R))
#define gp2x_video_color15(R,G,B,A) ((((R)&0xF8)<<8)|(((G)&0xF8)<<3)|(((B)&0xF8)>>3)|((A)<<5))

#define gp2x_video_getr15(C) (((C)>>8)&0xF8)
#define gp2x_video_getg15(C) (((C)>>3)&0xF8)
#define gp2x_video_getb15(C) (((C)<<3)&0xF8)

enum  { GP2X_UP=0x1,       GP2X_LEFT=0x4,       GP2X_DOWN=0x10,  GP2X_RIGHT=0x40,
        GP2X_START=1<<8,   GP2X_SELECT=1<<9,    GP2X_L=1<<10,    GP2X_R=1<<11,
        GP2X_A=1<<12,      GP2X_B=1<<13,        GP2X_X=1<<14,    GP2X_Y=1<<15,
        GP2X_VOL_UP=1<<23, GP2X_VOL_DOWN=1<<22, GP2X_PUSH=1<<27 };
                                            
extern volatile unsigned short 	gp2x_palette[512];
extern unsigned char 		*gp2x_screen8;
extern unsigned short 		*gp2x_screen15;
extern int			gp2x_sound_rate;
extern int			gp2x_sound_stereo;
extern int 			gp2x_pal_50hz;
extern int			gp2x_clock;

extern void gp2x_init(int tickspersecond, int bpp, int rate, int bits, int stereo, int hz);
extern void gp2x_deinit(void);
extern void gp2x_timer_delay(unsigned long ticks);
extern void gp2x_sound_play(void *buff, int len);
extern void gp2x_video_flip(void);
extern void gp2x_video_flip_single(void);
extern void gp2x_video_setpalette(void);
extern unsigned long gp2x_joystick_read(int n);
extern unsigned long gp2x_joystick_press (int n);
extern void gp2x_sound_volume(int left, int right);
extern void gp2x_sound_thread_mute(void);
extern void gp2x_sound_thread_start(void);
extern void gp2x_sound_thread_stop(void);
extern void gp2x_sound_set_rate(int rate);
extern void gp2x_sound_set_stereo(int stereo);
extern unsigned long gp2x_timer_read(void);
extern unsigned long gp2x_timer_read_real(void);
extern unsigned long gp2x_timer_read_scale(void);
extern void gp2x_timer_profile(void);

extern void gp2x_set_video_mode(int bpp,int width,int height);
extern void gp2x_set_clock(int mhz);

#ifdef MMUHACK
#include "squidgehack.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
void flushcache(void *start_address, void *end_address, int flags);
#ifdef __cplusplus
} /* End of extern "C" */
#endif

extern void set_ram_tweaks(void);

extern void gp2x_printf(char* fmt, ...);
extern void gp2x_printf_init(void);
extern void gp2x_gamelist_text_out(int x, int y, char *eltexto);
extern void gp2x_gamelist_text_out_fmt(int x, int y, char* fmt, ...);

#endif
