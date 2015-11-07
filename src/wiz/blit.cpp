#include "driver.h"
#include "dirty.h"

/* from video.c */
extern char *dirty_old;
extern char *dirty_new;
extern int gfx_xoffset;
extern int gfx_yoffset;
extern int gfx_display_lines;
extern int gfx_display_columns;
extern int gfx_width;
extern int gfx_height;
extern int skiplines;
extern int skipcolumns;

#define SCREEN8 fb1_8bit
#define SCREEN16 fb1_16bit
#define FLIP_VIDEO wiz_video_flip()

extern int video_scale;

#include "wiz_lib.h"

UINT32 *palette_16bit_lookup;

void blitscreen_dirty1_color8(struct osd_bitmap *bitmap)
{
	int x, y;
	int width=(bitmap->line[1] - bitmap->line[0]);
	unsigned char *lb=bitmap->line[skiplines] + skipcolumns;
	unsigned char *address=SCREEN8 + gfx_xoffset + (gfx_yoffset * gfx_width);

	for (y = 0; y < gfx_display_lines; y += 16)
	{
		for (x = 0; x < gfx_display_columns; )
		{
			int w = 16;
			if (ISDIRTY(x,y))
			{
				int h;
				unsigned char *lb0 = lb + x;
				unsigned char *address0 = address + x;
				while (x + w < gfx_display_columns && ISDIRTY(x+w,y))
                    			w += 16;
				if (x + w > gfx_display_columns)
                    			w = gfx_display_columns - x;
				for (h = 0; ((h < 16) && ((y + h) < gfx_display_lines)); h++)
				{
					memcpy(address0,lb0,w);
					lb0 += width;
					address0 += gfx_width;
				}
			}
			x += w;
        	}
		lb += 16 * width;
		address += 16 * gfx_width;
	}
}

INLINE void blitscreen_dirty0_color8_noscale(struct osd_bitmap *bitmap)
{
	int y=gfx_display_lines;
	int width=(bitmap->line[1] - bitmap->line[0]);
	int columns=gfx_display_columns;
	unsigned char *lb = bitmap->line[skiplines] + skipcolumns;
	unsigned char *address = SCREEN8 + gfx_xoffset + (gfx_yoffset * gfx_width);

    do
	{
		memcpy(address,lb,columns);
		lb+=width;
		address+=gfx_width;
		y--;
	}
	while (y);
	
	FLIP_VIDEO;
}

INLINE void blitscreen_dirty0_color8_horzscale(struct osd_bitmap *bitmap)
{
	unsigned char *buffer_scr = (unsigned char *)(SCREEN8+gfx_xoffset+(320*gfx_yoffset));
	unsigned char *buffer_mem = (unsigned char *)(bitmap->line[skiplines]+skipcolumns);
	int buffer_mem_offset = (bitmap->line[1] - bitmap->line[0])-gfx_width;
	int step,i;
	int x,y=gfx_display_lines;
	
	if (gfx_width>320)
	{
		/* Strech */
		step=320/(gfx_width-320);
		do {
			x=320; i=step;
			do {
				*buffer_scr++=*buffer_mem++;
				x--; i--;
				if (!i) { buffer_mem++; i=step; }
			} while (x);
			buffer_mem+=buffer_mem_offset;		
			y--;
		} while (y);
	}
	else
	{
		/* Scale */
		step=320/(320-gfx_width);
		do {
			x=320; i=1;
			do {
				i--;
				if (i) { *buffer_scr++=*buffer_mem++; }
				else { *buffer_scr++=*buffer_mem; i=step; }
				x--;
			} while (x);						
			buffer_mem+=buffer_mem_offset;		
			y--;
		} while (y);
	}
	
	FLIP_VIDEO;
}

INLINE void blitscreen_dirty0_color8_scale(struct osd_bitmap *bitmap)
{
    unsigned char *buffer_scr=SCREEN8;
    unsigned int W,H,ix,iy,x,y;
    x=0;
    y=0;
    W=320;
    H=240;
    ix=(gfx_display_columns<<16)/W;
    iy=(gfx_display_lines<<16)/H;
    
    do    
    {
        unsigned char *buffer_mem=bitmap->line[(y>>16)+skiplines]+skipcolumns;
        W=320; x=0;
        do {
            *buffer_scr++=buffer_mem[x>>16];
            x+=ix;
        } while (--W);
        y+=iy;
    } while (--H);
    
	FLIP_VIDEO;
}

void blitscreen_dirty0_color8(struct osd_bitmap *bitmap)
{
    if (video_scale)
    {
        if (video_scale==1)
            blitscreen_dirty0_color8_horzscale(bitmap);
        else
            blitscreen_dirty0_color8_scale(bitmap);
    }
    else
    {
        blitscreen_dirty0_color8_noscale(bitmap);
    }
}

void blitscreen_dirty1_palettized16(struct osd_bitmap *bitmap)
{
	int x, y;
	int width=(bitmap->line[1] - bitmap->line[0])>>1;
	unsigned short *lb=((unsigned short*)(bitmap->line[skiplines])) + skipcolumns;
	unsigned short *address=SCREEN16 + gfx_xoffset + (gfx_yoffset * gfx_width);

	for (y = 0; y < gfx_display_lines; y += 16)
	{
		for (x = 0; x < gfx_display_columns; )
		{
			int w = 16;
			if (ISDIRTY(x,y))
			{
				int h;
				unsigned short *lb0 = lb + x;
				unsigned short *address0 = address + x;
				while (x + w < gfx_display_columns && ISDIRTY(x+w,y))
                    			w += 16;
				if (x + w > gfx_display_columns)
                    			w = gfx_display_columns - x;
				for (h = 0; ((h < 16) && ((y + h) < gfx_display_lines)); h++)
				{
					int wx;
					for (wx=0;wx<w;wx++)
					{
						address0[wx] = palette_16bit_lookup[lb0[wx]];
					}
					lb0 += width;
					address0 += gfx_width;
				}
			}
			x += w;
        	}
		lb += 16 * width;
		address += 16 * gfx_width;
	}
}

INLINE unsigned int mix_color16 (unsigned int color1, unsigned int color2)
{
    return wiz_video_color16((wiz_video_getr16(color1)+wiz_video_getr16(color2))>>1,(wiz_video_getg16(color1)+wiz_video_getg16(color2))>>1,(wiz_video_getb16(color1)+wiz_video_getb16(color2))>>1);
}

INLINE void blitscreen_dirty0_palettized16_noscale(struct osd_bitmap *bitmap)
{
	int x,y;
	int width=(bitmap->line[1] - bitmap->line[0])>>1;
	int columns=gfx_display_columns;
	unsigned short *lb = ((unsigned short*)(bitmap->line[skiplines])) + skipcolumns;
	unsigned short *address = SCREEN16 + gfx_xoffset + (gfx_yoffset * gfx_width);

	for (y = 0; y < gfx_display_lines; y++)
	{
		for (x = 0; x < columns; x++)
		{
			address[x] = palette_16bit_lookup[lb[x]];
		}
		lb+=width;
		address+=gfx_width;
	}
	
	FLIP_VIDEO;
}

INLINE void blitscreen_dirty0_palettized16_horzscale(struct osd_bitmap *bitmap)
{
	unsigned short *buffer_scr = SCREEN16 + gfx_xoffset + (320*gfx_yoffset);
	unsigned short *buffer_mem = ((unsigned short*)(bitmap->line[skiplines])) + skipcolumns;
	int buffer_mem_offset = ((bitmap->line[1] - bitmap->line[0])>>1)-gfx_width;
	int step,i;
	int x,y=gfx_display_lines;
	
	if (gfx_width>320)
	{
		/* Strech */
		step=320/(gfx_width-320);
		do {
			x=320; i=step;
			do {
				*buffer_scr++=palette_16bit_lookup[*buffer_mem++];
				x--; i--;
				if (!i) { if (x) { *buffer_scr++=mix_color16(palette_16bit_lookup[*buffer_mem++],palette_16bit_lookup[*buffer_mem++]); x--; i=step-1; } else { buffer_mem++; i=step; } }
			} while (x);
			buffer_mem+=buffer_mem_offset;		
			y--;
		} while (y);
	}
	else
	{
		/* Scale */
		step=320/(320-gfx_width);
		do {
			x=320; i=1;
			do {
				i--;
				if (i) { *buffer_scr++=palette_16bit_lookup[*buffer_mem++]; }
				else { *buffer_scr++=mix_color16(palette_16bit_lookup[buffer_mem[0]],palette_16bit_lookup[buffer_mem[-1]]); i=step; }
				x--;
			} while (x);						
			buffer_mem+=buffer_mem_offset;		
			y--;
		} while (y);
	}
	
	FLIP_VIDEO;
}

INLINE void blitscreen_dirty0_palettized16_scale(struct osd_bitmap *bitmap)
{
    unsigned short *buffer_scr=SCREEN16;
    unsigned int W,H,ix,iy,x,y;
    x=0;
    y=0;
    W=320;
    H=240;
    ix=(gfx_display_columns<<16)/W;
    iy=(gfx_display_lines<<16)/H;
    
    do    
    {
        unsigned short *buffer_mem=((unsigned short *)(bitmap->line[(y>>16)+skiplines]))+skipcolumns;
        W=320; x=0;
        do {
            *buffer_scr++=palette_16bit_lookup[buffer_mem[x>>16]];
            x+=ix;
        } while (--W);
        y+=iy;
    } while (--H);
    
	FLIP_VIDEO;
}

void blitscreen_dirty0_palettized16(struct osd_bitmap *bitmap)
{
    if (video_scale)
    {
        if (video_scale==1)
            blitscreen_dirty0_palettized16_horzscale(bitmap);
        else
            blitscreen_dirty0_palettized16_scale(bitmap);
    }
    else
    {
        blitscreen_dirty0_palettized16_noscale(bitmap);
    }
}

void blitscreen_dirty1_color16(struct osd_bitmap *bitmap)
{
	int x, y;
	int width=(bitmap->line[1] - bitmap->line[0])>>1;
	unsigned short *lb=((unsigned short*)(bitmap->line[skiplines])) + skipcolumns;
	unsigned short *address=SCREEN16 + gfx_xoffset + (gfx_yoffset * gfx_width);

	for (y = 0; y < gfx_display_lines; y += 16)
	{
		for (x = 0; x < gfx_display_columns; )
		{
			int w = 16;
			if (ISDIRTY(x,y))
			{
				int h;
				unsigned short *lb0 = lb + x;
				unsigned short *address0 = address + x;
				while (x + w < gfx_display_columns && ISDIRTY(x+w,y))
                    			w += 16;
				if (x + w > gfx_display_columns)
                    			w = gfx_display_columns - x;
				for (h = 0; ((h < 16) && ((y + h) < gfx_display_lines)); h++)
				{
					memcpy(address0,lb0,w<<1);
					lb0 += width;
					address0 += gfx_width;
				}
			}
			x += w;
        	}
		lb += 16 * width;
		address += 16 * gfx_width;
	}
}

INLINE void blitscreen_dirty0_color16_noscale(struct osd_bitmap *bitmap)
{
	int y=gfx_display_lines;
	int width=(bitmap->line[1] - bitmap->line[0])>>1;
	int columns=gfx_display_columns<<1;
	unsigned short *lb = ((unsigned short*)(bitmap->line[skiplines])) + skipcolumns;
	unsigned short *address = SCREEN16 + gfx_xoffset + (gfx_yoffset * gfx_width);

	do
	{
	    memcpy(address,lb,columns);
		lb+=width;
		address+=gfx_width;
		y--;
	}
	while (y);
	
	FLIP_VIDEO;
}

INLINE void blitscreen_dirty0_color16_horzscale(struct osd_bitmap *bitmap)
{
	unsigned short *buffer_scr = SCREEN16 + gfx_xoffset + (320*gfx_yoffset);
	unsigned short *buffer_mem = ((unsigned short*)(bitmap->line[skiplines])) + skipcolumns;
	int buffer_mem_offset = ((bitmap->line[1] - bitmap->line[0])>>1)-gfx_width;
	int step,i;
	int x,y=gfx_display_lines;
	
	if (gfx_width>320)
	{
		/* Strech */
		step=320/(gfx_width-320);
		do {
			x=320; i=step;
			do {
				*buffer_scr++=*buffer_mem++;
				x--; i--;
				if (!i) { if (x) { *buffer_scr++=mix_color16(*buffer_mem++,*buffer_mem++); x--; i=step-1; } else { buffer_mem++; i=step; } }
			} while (x);
			buffer_mem+=buffer_mem_offset;		
			y--;
		} while (y);
	}
	else
	{
		/* Scale */
		step=320/(320-gfx_width);
		do {
			x=320; i=1;
			do {
				i--;
				if (i) { *buffer_scr++=*buffer_mem++; }
				else { *buffer_scr++=mix_color16(buffer_mem[0],buffer_mem[-1]); i=step; }
				x--;
			} while (x);						
			buffer_mem+=buffer_mem_offset;		
			y--;
		} while (y);
	}
	
	FLIP_VIDEO;
}

INLINE void blitscreen_dirty0_color16_scale(struct osd_bitmap *bitmap)
{
    unsigned short *buffer_scr=SCREEN16;
    unsigned int W,H,ix,iy,x,y;
    x=0;
    y=0;
    W=320;
    H=240;
    ix=(gfx_display_columns<<16)/W;
    iy=(gfx_display_lines<<16)/H;
    
    do    
    {
        unsigned short *buffer_mem=((unsigned short *)(bitmap->line[(y>>16)+skiplines]))+skipcolumns;
        W=320; x=0;
        do {
            *buffer_scr++=buffer_mem[x>>16];
            x+=ix;
        } while (--W);
        y+=iy;
    } while (--H);
    
	FLIP_VIDEO;
}

void blitscreen_dirty0_color16(struct osd_bitmap *bitmap)
{
    if (video_scale)
    {
        if (video_scale==1)
            blitscreen_dirty0_color16_horzscale(bitmap);
        else
            blitscreen_dirty0_color16_scale(bitmap);
    }
    else
    {
        blitscreen_dirty0_color16_noscale(bitmap);
    }
}
