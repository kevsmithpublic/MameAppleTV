#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "minimal.h"
#include "gp2x_frontend_list.h"
#include "gp2x_frontend_menu.h"
#include "gp2x_frontend_splash.h"

int game_num_avail=0;
static int last_game_selected=0;
char playemu[16] = "mame\0";
char playgame[16] = "builtinn\0";

int gp2x_freq=200;
int gp2x_video_depth=8;
int gp2x_video_aspect=0;
int gp2x_video_sync=0;
int gp2x_frameskip=-1;
int gp2x_sound = 1;
int gp2x_volume = 3;
int gp2x_clock_cpu=80;
int gp2x_clock_sound=80;
int gp2x_cpu_cores=1;
int gp2x_ramtweaks=1;
int gp2x_cheat=0;

int master_volume = 100;

static void load_bmp_8bpp(unsigned char *out, unsigned char *in) 
{
	int i,x,y;
	unsigned char r,g,b,c;
	in+=14; /* Skip HEADER */
	in+=40; /* Skip INFOHD */
	/* Set Palette */
	for (i=0;i<256;i++) {
		b=*in++;
		g=*in++;
		r=*in++;
		c=*in++;
		gp2x_video_color8(i,r,g,b);
	}
	gp2x_video_setpalette();
	/* Set Bitmap */	
	for (y=239;y!=-1;y--) {
		for (x=0;x<320;x++) {
			*out++=in[x+y*320];
		}
	}
}

static void gp2x_intro_screen(void) {
	char name[256];
	FILE *f;
	gp2x_video_flip();
	sprintf(name,"skins/gp2xsplash.bmp");
	f=fopen(name,"rb");
	if (f) {
		fread(gp2xsplash_bmp,1,77878,f);
		fclose(f);
	}
	load_bmp_8bpp(gp2x_screen8,gp2xsplash_bmp);
	gp2x_video_flip();
	sprintf(name,"skins/gp2xmenu.bmp");
	f=fopen(name,"rb");
	if (f) {
		fread(gp2xmenu_bmp,1,77878,f);
		fclose(f);
	}
}

static void game_list_init_nocache(void)
{
	int i;
	FILE *f;
	DIR *d=opendir("roms");
	char game[32];
	if (d)
	{
		struct dirent *actual=readdir(d);
		while(actual)
		{
			for (i=0;i<NUMGAMES;i++)
			{
				if (drivers[i].available==0)
				{
					sprintf(game,"%s.zip",drivers[i].name);
					if (strcmp(actual->d_name,game)==0)
					{
						drivers[i].available=1;
						game_num_avail++;
						break;
					}
				}
			}
			actual=readdir(d);
		}
		closedir(d);
	}
	
	if (game_num_avail)
	{
		remove("frontend/mame.lst");
		sync();
		f=fopen("frontend/mame.lst","w");
		if (f)
		{
			for (i=0;i<NUMGAMES;i++)
			{
				fputc(drivers[i].available,f);
			}
			fclose(f);
			sync();
		}
	}
}

static void game_list_init_cache(void)
{
	FILE *f;
	int i;
	f=fopen("frontend/mame.lst","r");
	if (f)
	{
		for (i=0;i<NUMGAMES;i++)
		{
			drivers[i].available=fgetc(f);
			if (drivers[i].available)
				game_num_avail++;
		}
		fclose(f);
	}
	else
		game_list_init_nocache();
}

static void game_list_init(int argc)
{
	if (argc==1)
		game_list_init_nocache();
	else
		game_list_init_cache();
}

static void game_list_view(int *pos) {

	int i;
	int view_pos;
	int aux_pos=0;
	int screen_y = 45;
	int screen_x = 38;

	/* Draw background image */
	load_bmp_8bpp(gp2x_screen8,gp2xmenu_bmp);

	/* Check Limits */
	if (*pos<0)
		*pos=game_num_avail-1;
	if (*pos>(game_num_avail-1))
		*pos=0;
					   
	/* Set View Pos */
	if (*pos<10) {
		view_pos=0;
	} else {
		if (*pos>game_num_avail-11) {
			view_pos=game_num_avail-21;
			view_pos=(view_pos<0?0:view_pos);
		} else {
			view_pos=*pos-10;
		}
	}

	/* Show List */
	for (i=0;i<NUMGAMES;i++) {
		if (drivers[i].available==1) {
			if (aux_pos>=view_pos && aux_pos<=view_pos+20) {
				gp2x_gamelist_text_out( screen_x, screen_y, drivers[i].description);
				if (aux_pos==*pos) {
					gp2x_gamelist_text_out( screen_x-10, screen_y,">" );
					gp2x_gamelist_text_out( screen_x-13, screen_y-1,"-" );
				}
				screen_y+=8;
			}
			aux_pos++;
		}
	}
}

static void game_list_select (int index, char *game, char *emu) {
	int i;
	int aux_pos=0;
	for (i=0;i<NUMGAMES;i++)
	{
		if (drivers[i].available==1)
		{
			if(aux_pos==index)
			{
				strcpy(game,drivers[i].name);
				strcpy(emu,drivers[i].exe);
				gp2x_cpu_cores=drivers[i].cores;
				break;
			}
			aux_pos++;
		}
	}
}

static char *game_list_description (int index)
{
	int i;
	int aux_pos=0;
	for (i=0;i<NUMGAMES;i++) {
		if (drivers[i].available==1) {
			if(aux_pos==index) {
				return(drivers[i].description);
			}
			aux_pos++;
		   }
	}
	return ((char *)0);
}

static int show_options(char *game)
{
	unsigned long ExKey=0;
	int selected_option=0;
	int x_Pos = 41;
	int y_Pos = 58;
	int options_count = 12;
	char text[256];
	FILE *f;
	int i=0;

	/* Read game configuration */
	sprintf(text,"frontend/%s.cfg",game);
	f=fopen(text,"r");
	if (f) {
		fscanf(f,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&gp2x_freq,&gp2x_video_depth,&gp2x_video_aspect,&gp2x_video_sync,
		&gp2x_frameskip,&gp2x_sound,&gp2x_clock_cpu,&gp2x_clock_sound,&gp2x_cpu_cores,&gp2x_ramtweaks,&i,&gp2x_cheat,&gp2x_volume);
		fclose(f);
	}

	while(1)
	{
		/* Draw background image */
		load_bmp_8bpp(gp2x_screen8,gp2xmenu_bmp);

		/* Draw the options */
		strncpy (text,game_list_description(last_game_selected),33);
		text[32]='\0';
		gp2x_gamelist_text_out(x_Pos,y_Pos-10,text);

		/* (0) GP2X Clock */
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+10, "GP2X Clock    %d MHz", gp2x_freq);

		/* (1) Video Depth */
		switch (gp2x_video_depth)
		{
			case -1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+20,"Video Depth   Auto"); break;
			case 8:  gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+20,"Video Depth   8 bit"); break;
			case 16: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+20,"Video Depth   16 bit"); break;
		}

		/* (2) Video Aspect */
		switch (gp2x_video_aspect)
		{
			case 0: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Normal"); break;
			case 1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Scale"); break;
			case 2: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  4:3"); break;
			case 3: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Scale 4:3"); break;
			case 4: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Border"); break;
			case 5: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Border Scale"); break;
			case 6: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Border 4:3"); break;
			case 7: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Border Scale 4:3"); break;
			case 8: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Rotate"); break;
			case 9: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Rotate Scale"); break;
			case 10: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Rotate 4:3"); break;
			case 11: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Rotate Scale 4:3"); break;
			case 12: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Rotate Border"); break;
			case 13: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Rotate Border Scale"); break;
			case 14: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Rotate Border 4:3"); break;
			case 15: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Rotate Border S4:3"); break;
			case 16: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  TATE"); break;
			case 17: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  TATE Scale"); break;
			case 18: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  TATE 4:3"); break;
			case 19: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  TATE Scale 4:3"); break;
			case 20: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  TATE Border"); break;
			case 21: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  TATE Border Scale"); break;
			case 22: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  TATE Border 4:3"); break;
			case 23: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  TATE Border S4:3"); break;
		}
		
		/* (3) Video Sync */
		switch (gp2x_video_sync)
		{
			case 1: gp2x_gamelist_text_out(x_Pos,y_Pos+40, "Video Sync    VSync"); break;
			case 0: gp2x_gamelist_text_out(x_Pos,y_Pos+40, "Video Sync    Normal"); break;
			case 2: gp2x_gamelist_text_out(x_Pos,y_Pos+40, "Video Sync    DblBuf"); break;
			case -1: gp2x_gamelist_text_out(x_Pos,y_Pos+40,"Video Sync    OFF"); break;
		}
		
		/* (4) Frame-Skip */
		if ((gp2x_video_sync==-1) && (gp2x_frameskip==-1)) gp2x_frameskip=0;
		if(gp2x_frameskip==-1) {
			gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+50, "Frame-Skip    Auto");
		}
		else{
			gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+50,"Frame-Skip    %d",gp2x_frameskip);
		}

		/* (5) Sound */
		switch(gp2x_sound)
		{
			case 0: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","OFF"); break;
			case 1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (15 KHz fast)"); break;
			case 2: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (22 KHz fast)"); break;
			case 3: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (33 KHz fast)"); break;
			case 4: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (44 KHz fast)"); break;
			case 5: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (11 KHz fast)"); break;
			case 6: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (15 KHz)"); break;
			case 7: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (22 KHz)"); break;
			case 8: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (33 KHz)"); break;
			case 9: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (44 KHz)"); break;
			case 10: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (11 KHz)"); break;
			case 11: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (15 KHz stereo)"); break;
			case 12: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (22 KHz stereo)"); break;
			case 13: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (33 KHz stereo)"); break;
			case 14: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (44 KHz stereo)"); break;
			case 15: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (11 KHz stereo)"); break;
		}

		/* (6) CPU Clock */
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70,"CPU Clock     %d%%",gp2x_clock_cpu);

		/* (7) Audio Clock */
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Audio Clock   %d%%",gp2x_clock_sound);

		/* (8) CPU cores */
		switch (gp2x_cpu_cores)
		{
			case 0: gp2x_gamelist_text_out(x_Pos,y_Pos+90, "CPU ASM cores None"); break;
			case 1: gp2x_gamelist_text_out(x_Pos,y_Pos+90, "CPU ASM cores Cyclone"); break;
			case 2: gp2x_gamelist_text_out(x_Pos,y_Pos+90, "CPU ASM cores DrZ80"); break;
			case 3: gp2x_gamelist_text_out(x_Pos,y_Pos+90, "CPU ASM cores Cyclone+DrZ80"); break;
			case 4: gp2x_gamelist_text_out(x_Pos,y_Pos+90, "CPU ASM cores DrZ80(snd)"); break;
			case 5: gp2x_gamelist_text_out(x_Pos,y_Pos+90, "CPU ASM cores Cyclone+DrZ80(snd)"); break;
		}

		/* (9) RAM Tweaks */
		if (gp2x_ramtweaks)
			gp2x_gamelist_text_out(x_Pos,y_Pos+100,"RAM Tweaks    ON");
		else
			gp2x_gamelist_text_out(x_Pos,y_Pos+100,"RAM Tweaks    OFF");

		/* (10) Cheats */
		if (gp2x_cheat)
			gp2x_gamelist_text_out(x_Pos,y_Pos+110,"Cheats        ON");
		else
			gp2x_gamelist_text_out(x_Pos,y_Pos+110,"Cheats        OFF");

        /* (11) Volume */
        if (gp2x_sound == 0)
        {
            gp2x_gamelist_text_out(x_Pos,y_Pos+120,"Volume        Disabled");
        }
        else
        {
            switch (gp2x_volume)
            {
                case 1: gp2x_gamelist_text_out(x_Pos,y_Pos+120,"Volume        Quiet"); break;
                case 2: gp2x_gamelist_text_out(x_Pos,y_Pos+120,"Volume        Low"); break;
                case 3: gp2x_gamelist_text_out(x_Pos,y_Pos+120,"Volume        Medium"); break;
                case 4: gp2x_gamelist_text_out(x_Pos,y_Pos+120,"Volume        Maximum"); break;
            }
        }
	
		gp2x_gamelist_text_out(x_Pos,y_Pos+140,"Press B to confirm, X to return\0");

		/* Show currently selected item */
		gp2x_gamelist_text_out(x_Pos-16,y_Pos+(selected_option*10)+10," >");

		gp2x_video_flip();
		while(gp2x_joystick_read(0)&0x8c0ff55) { gp2x_timer_delay(150); }
		while(!(ExKey=gp2x_joystick_read(0)&0x8c0ff55)) { }
		if(ExKey & GP2X_DOWN){
			selected_option++;
			selected_option = selected_option % options_count;
		}
		else if(ExKey & GP2X_UP){
			selected_option--;
			if(selected_option<0)
				selected_option = options_count - 1;
		}
		else if(ExKey & GP2X_R || ExKey & GP2X_L)
		{
			switch(selected_option) {
			case 0:
				/* GP2X Clock */
				if(ExKey & GP2X_R){
					switch (gp2x_freq) {
						case  66: gp2x_freq=75;break;
						case  75: gp2x_freq=80;break;
						case  80: gp2x_freq=100;break;
						case 100: gp2x_freq=120;break;
						case 120: gp2x_freq=133;break;
						case 133: gp2x_freq=150;break;
						case 150: gp2x_freq=166;break;
						case 166: gp2x_freq=180;break;
						case 180: gp2x_freq=200;break;
						case 200: gp2x_freq=225;break;
						case 225: gp2x_freq=233;break;
						case 233: gp2x_freq=245;break;
						case 245: gp2x_freq=250;break;
						case 250: gp2x_freq=255;break;
						case 255: gp2x_freq=260;break;
						case 260: gp2x_freq=265;break;
						case 265: gp2x_freq=266;break;
						case 266: gp2x_freq=270;break;
						case 270: gp2x_freq=275;break;
						case 275: gp2x_freq=280;break;
						case 280: gp2x_freq=285;break;
						case 285: gp2x_freq=290;break;
						case 290: gp2x_freq=295;break;
						case 295: gp2x_freq=300;break;
						case 300: gp2x_freq=66;break;
					}
				} else {
					switch (gp2x_freq) {
						case  66: gp2x_freq=300;break;
						case  75: gp2x_freq=66;break;
						case  80: gp2x_freq=75;break;
						case 100: gp2x_freq=80;break;
						case 120: gp2x_freq=100;break;
						case 133: gp2x_freq=120;break;
						case 150: gp2x_freq=133;break;
						case 166: gp2x_freq=150;break;
						case 180: gp2x_freq=166;break;
						case 200: gp2x_freq=180;break;
						case 225: gp2x_freq=200;break;
						case 233: gp2x_freq=225;break;
						case 245: gp2x_freq=233;break;
						case 250: gp2x_freq=245;break;
						case 255: gp2x_freq=250;break;
						case 260: gp2x_freq=255;break;
						case 265: gp2x_freq=260;break;
						case 266: gp2x_freq=265;break;
						case 270: gp2x_freq=266;break;
						case 275: gp2x_freq=270;break;
						case 280: gp2x_freq=275;break;
						case 285: gp2x_freq=280;break;
						case 290: gp2x_freq=285;break;
						case 295: gp2x_freq=290;break;
						case 300: gp2x_freq=295;break;
					}
				}
				break;
			case 1:
				switch (gp2x_video_depth)
				{
					case -1: gp2x_video_depth=8; break;
					case 8: gp2x_video_depth=16; break;
					case 16: gp2x_video_depth=-1; break;
				}
				break;
			case 2:
				if(ExKey & GP2X_R)
				{
					gp2x_video_aspect++;
					if (gp2x_video_aspect>23)
						gp2x_video_aspect=0;
				}
				else
				{
					gp2x_video_aspect--;
					if (gp2x_video_aspect<0)
						gp2x_video_aspect=23;
				}
				break;
			case 3:
				gp2x_video_sync=gp2x_video_sync+1;
				if (gp2x_video_sync>2)
					gp2x_video_sync=-1;
				break;
			case 4:
				/* "Frame-Skip" */
				if(ExKey & GP2X_R)
				{
					gp2x_frameskip ++;
					if (gp2x_frameskip>11)
						gp2x_frameskip=-1;
				}
				else
				{
					gp2x_frameskip--;
					if (gp2x_frameskip<-1)
						gp2x_frameskip=11;
				}
				break;
			case 5:
				if(ExKey & GP2X_R)
				{
					gp2x_sound ++;
					if (gp2x_sound>15)
						gp2x_sound=0;
				}
				else
				{
					gp2x_sound--;
					if (gp2x_sound<0)
						gp2x_sound=15;
				}
				break;
			case 6:
				/* "CPU Clock" */
				if(ExKey & GP2X_R)
				{
					gp2x_clock_cpu += 10; /* Add 10% */
					if (gp2x_clock_cpu > 200) /* 200% is the max */
						gp2x_clock_cpu = 200;
				}
				else
				{
					gp2x_clock_cpu -= 10; /* Subtract 10% */
					if (gp2x_clock_cpu < 10) /* 10% is the min */
						gp2x_clock_cpu = 10;
				}
				break;
			case 7:
				/* "Audio Clock" */
				if(ExKey & GP2X_R)
				{
					gp2x_clock_sound += 10; /* Add 10% */
					if (gp2x_clock_sound > 200) /* 200% is the max */
						gp2x_clock_sound = 200;
				}
				else{
					gp2x_clock_sound -= 10; /* Subtract 10% */
					if (gp2x_clock_sound < 10) /* 10% is the min */
						gp2x_clock_sound = 10;
				}
				break;
			case 8:
				gp2x_cpu_cores=(gp2x_cpu_cores+1)%6;
				break;
			case 9:
				gp2x_ramtweaks=!gp2x_ramtweaks;
				break;
			case 10:
				gp2x_cheat=!gp2x_cheat;
				break;
            case 11:
                /* Volume */
                if(ExKey & GP2X_R)
                {
                    gp2x_volume++;
                    if (gp2x_volume > 4)
                        gp2x_volume = 1;
                }
                else {
                    gp2x_volume--;
                    if (gp2x_volume < 1)
                        gp2x_volume = 4;
                }
                break;
			}
		}

		if ((ExKey & GP2X_A) || (ExKey & GP2X_B) || (ExKey & GP2X_PUSH) || (ExKey & GP2X_START)) 
		{
			/* Write game configuration */
			sprintf(text,"frontend/%s.cfg",game);
			f=fopen(text,"w");
			if (f) {
				fprintf(f,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",gp2x_freq,gp2x_video_depth,gp2x_video_aspect,gp2x_video_sync,
				gp2x_frameskip,gp2x_sound,gp2x_clock_cpu,gp2x_clock_sound,gp2x_cpu_cores,gp2x_ramtweaks,i,gp2x_cheat,gp2x_volume);
				fclose(f);
				sync();
			}

			/* Selected game will be run */
			return 1;
		}
		else if ((ExKey & GP2X_X) || (ExKey & GP2X_Y) || (ExKey & GP2X_SELECT))
		{
			/* Return To Menu */
			return 0;
		}
	}
}

static void gp2x_exit(void)
{
	remove("frontend/mame.lst");
	sync();
	gp2x_deinit();
	chdir("/usr/gp2x"); /*go to menu*/
      	execl("gp2xmenu", "gp2xmenu", NULL);
}

static void select_game(char *emu, char *game)
{

	unsigned long ExKey;

	/* No Selected game */
	strcpy(game,"builtinn");

	/* Clean screen */
	gp2x_video_flip();

	/* Wait until user selects a game */
	while(1)
	{
		game_list_view(&last_game_selected);
		gp2x_video_flip();

		if( (gp2x_joystick_read(0)&0x8c0ff55)) 
			gp2x_timer_delay(100); 
		while(!(ExKey=gp2x_joystick_read(0)&0x8c0ff55))
		{ 
			if ((ExKey & GP2X_L) && (ExKey & GP2X_R)) { gp2x_exit(); }
		}

		if (ExKey & GP2X_UP) last_game_selected--;
		if (ExKey & GP2X_DOWN) last_game_selected++;
		if (ExKey & GP2X_L) last_game_selected-=21;
		if (ExKey & GP2X_R) last_game_selected+=21;
		if ((ExKey & GP2X_L) && (ExKey & GP2X_R)) gp2x_exit();

		if ((ExKey & GP2X_A) || (ExKey & GP2X_B) || (ExKey & GP2X_PUSH) || (ExKey & GP2X_START))
		{
			/* Select the game */
			game_list_select(last_game_selected, game, emu);

			/* Emulation Options */
			if(show_options(game))
			{
				break;
			}
		}
	}
}

void execute_game (char *playemu, char *playgame)
{
	char *args[255];
	char str[8][64];
	int n=0;
	int i=0;
	
	/* executable */
	args[n]=playemu; n++;

	/* playgame */
	args[n]=playgame; n++;

	/* gp2x_freq */
	args[n]="-clock"; n++;
	sprintf(str[i],"%d",gp2x_freq);
	args[n]=str[i]; i++; n++;

	/* gp2x_video_depth */
	if (gp2x_video_depth==8)
	{
		args[n]="-depth"; n++;
		args[n]="8"; n++;
	}
	if (gp2x_video_depth==16)
	{
		args[n]="-depth"; n++;
		args[n]="16"; n++;
	}
	//
	//args[n]="-scale"; n++;
	//
	/* gp2x_video_aspect */
	if ((gp2x_video_aspect==1) || (gp2x_video_aspect==3) || (gp2x_video_aspect==5) || (gp2x_video_aspect==7) ||
		(gp2x_video_aspect==9) || (gp2x_video_aspect==11) || (gp2x_video_aspect==13) || (gp2x_video_aspect==15) ||
		(gp2x_video_aspect==17) || (gp2x_video_aspect==19) || (gp2x_video_aspect==21) || (gp2x_video_aspect==23))
	{
		args[n]="-scale"; n++;
	}
	if ((gp2x_video_aspect==2) || (gp2x_video_aspect==3) || (gp2x_video_aspect==6) || (gp2x_video_aspect==7) ||
		(gp2x_video_aspect==10) || (gp2x_video_aspect==11) || (gp2x_video_aspect==14) || (gp2x_video_aspect==15) ||
		(gp2x_video_aspect==18) || (gp2x_video_aspect==19) || (gp2x_video_aspect==22) || (gp2x_video_aspect==23))
	{
		args[n]="-aspect"; n++;
	}
	if ((gp2x_video_aspect==4) || (gp2x_video_aspect==5) || (gp2x_video_aspect==6) || (gp2x_video_aspect==7) ||
		(gp2x_video_aspect==12) || (gp2x_video_aspect==13) || (gp2x_video_aspect==14) || (gp2x_video_aspect==15) ||
		(gp2x_video_aspect==20) || (gp2x_video_aspect==21) || (gp2x_video_aspect==22) || (gp2x_video_aspect==23))
	{
		args[n]="-border"; n++;
	}
	if ((gp2x_video_aspect>=8) && (gp2x_video_aspect<=23))
	{
		args[n]="-ror"; n++;
	}
	if ((gp2x_video_aspect>=8) && (gp2x_video_aspect<=15))
	{
		args[n]="-rotatecontrols"; n++;
	}
	
	/* gp2x_video_sync */
	if (gp2x_video_sync==1)
	{
		args[n]="-nodirty"; n++;
		args[n]="-waitvsync"; n++;
	}
	else if (gp2x_video_sync==2)
	{
		args[n]="-nodirty"; n++;
	}
	else if (gp2x_video_sync==-1)
	{
		args[n]="-nothrottle"; n++;
	}
	
	/* gp2x_frameskip */
	if (gp2x_frameskip>=0)
	{
		args[n]="-frameskip"; n++;
		sprintf(str[i],"%d",gp2x_frameskip);
		args[n]=str[i]; i++; n++;
	}

	/* gp2x_sound */
	if (gp2x_sound==0)
	{
		args[n]="-soundcard"; n++;
		args[n]="0"; n++;
	}
	if ((gp2x_sound==1) || (gp2x_sound==6) || (gp2x_sound==11))
	{
		args[n]="-samplerate"; n++;
		args[n]="16000"; n++;
	}
	if ((gp2x_sound==2) || (gp2x_sound==7) || (gp2x_sound==12))
	{
		args[n]="-samplerate"; n++;
		args[n]="22050"; n++;
	}
	if ((gp2x_sound==3) || (gp2x_sound==8) || (gp2x_sound==13))
	{
		args[n]="-samplerate"; n++;
		args[n]="32000"; n++;
	}
	if ((gp2x_sound==4) || (gp2x_sound==9) || (gp2x_sound==14))
	{
		args[n]="-samplerate"; n++;
		args[n]="44100"; n++;
	}
	if ((gp2x_sound==5) || (gp2x_sound==10) || (gp2x_sound==15))
	{
		args[n]="-samplerate"; n++;
		args[n]="11025"; n++;
	}
	if ((gp2x_sound>=1) && (gp2x_sound<=5))
	{
		args[n]="-fastsound"; n++;
	}
	if (gp2x_sound>=11)
	{
		args[n]="-stereo"; n++;
	}

	/* gp2x_clock_cpu */
	if (gp2x_clock_cpu!=100)
	{
		args[n]="-uclock"; n++;
		sprintf(str[i],"%d",100-gp2x_clock_cpu);
		args[n]=str[i]; i++; n++;
	}

	/* gp2x_clock_sound */
	if (gp2x_clock_cpu!=100)
	{
		args[n]="-uclocks"; n++;
		sprintf(str[i],"%d",100-gp2x_clock_sound);
		args[n]=str[i]; i++; n++;
	}
	
	/* gp2x_cpu_cores */
	if ((gp2x_cpu_cores==1) || (gp2x_cpu_cores==3) || (gp2x_cpu_cores==5))
	{
		args[n]="-cyclone"; n++;
	}
	if ((gp2x_cpu_cores==2) || (gp2x_cpu_cores==3))
	{
		args[n]="-drz80"; n++;
	}
	if ((gp2x_cpu_cores==4) || (gp2x_cpu_cores==5))
	{
		args[n]="-drz80_snd"; n++;
	}

	if (gp2x_ramtweaks)
	{
		args[n]="-ramtweaks"; n++;
	}
	
	if (gp2x_cheat)
	{
		args[n]="-cheat"; n++;
	}

	switch (gp2x_volume)
	{
	    case 4: break; /* nothing, default to maximum volume */
		case 3: args[n]="-volume"; n++; args[n]="-4"; n++; break;
		case 2: args[n]="-volume"; n++; args[n]="-8"; n++; break;
		case 1: args[n]="-volume"; n++; args[n]="-10"; n++; break;
	}

	args[n]=NULL;
	
	for (i=0; i<n; i++)
	{
		printf("%s ",args[i]);
	}
	printf("\n");
	
	gp2x_deinit();
	execv(args[0], args); 
}


int main (int argc, char **argv)
{
	FILE *f;

	/* GP2X Initialization */
	gp2x_init(1000,8,22050,16,0,60);

	/* Show intro screen */
	gp2x_intro_screen();

	/* Initialize list of available games */
	game_list_init(argc);
	if (game_num_avail==0)
	{
		gp2x_gamelist_text_out(35, 110, "ERROR: NO AVAILABLE GAMES FOUND");
		gp2x_video_flip();
		gp2x_joystick_press(0);
		gp2x_exit();
	}

	/* Read default configuration */
	f=fopen("frontend/mame.cfg","r");
	if (f) {
		fscanf(f,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&gp2x_freq,&gp2x_video_depth,&gp2x_video_aspect,&gp2x_video_sync,
		&gp2x_frameskip,&gp2x_sound,&gp2x_clock_cpu,&gp2x_clock_sound,&gp2x_cpu_cores,&gp2x_ramtweaks,&last_game_selected,&gp2x_cheat,&gp2x_volume);
		fclose(f);
	}
	
	/* Select Game */
	select_game(playemu,playgame); 

	/* Write default configuration */
	f=fopen("frontend/mame.cfg","w");
	if (f) {
		fprintf(f,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",gp2x_freq,gp2x_video_depth,gp2x_video_aspect,gp2x_video_sync,
		gp2x_frameskip,gp2x_sound,gp2x_clock_cpu,gp2x_clock_sound,gp2x_cpu_cores,gp2x_ramtweaks,last_game_selected,gp2x_cheat,gp2x_volume);
		fclose(f);
		sync();
	}
	
	/* Execute Game */
	execute_game (playemu,playgame);
	
	exit (0);
}
