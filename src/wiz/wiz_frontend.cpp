#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "wiz_lib.h"
#include "wiz_frontend_list.h"
#include "wiz_frontend_menu.h"
#include "wiz_frontend_splash.h"

int game_num_avail=0;
static int last_game_selected=0;
char playemu[16] = "mame\0";
char playgame[16] = "builtinn\0";

int wiz_freq=533;
int wiz_video_depth=8;
int wiz_video_aspect=0;
int wiz_video_sync=0;
int wiz_frameskip=-1;
int wiz_sound = 2;
int wiz_volume = 3;
int wiz_clock_cpu=80;
int wiz_clock_sound=80;
int wiz_cpu_cores=1;
int wiz_ramtweaks=1;
int wiz_cheat=0;

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
		wiz_video_color8(i,r,g,b);
	}
	wiz_video_setpalette();
	/* Set Bitmap */	
	for (y=239;y!=-1;y--) {
		for (x=0;x<320;x++) {
			*out++=in[x+y*320];
		}
	}
}

static void wiz_intro_screen(void) {
	char name[256];
	FILE *f;
	wiz_video_flip();
	sprintf(name,"skins/wizsplash.bmp");
	f=fopen(name,"rb");
	if (f) {
		fread(wizsplash_bmp,1,77878,f);
		fclose(f);
	}
	load_bmp_8bpp(fb1_8bit,wizsplash_bmp);
	wiz_video_flip();
	sprintf(name,"skins/wizmenu.bmp");
	f=fopen(name,"rb");
	if (f) {
		fread(wizmenu_bmp,1,77878,f);
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
	load_bmp_8bpp(fb1_8bit,wizmenu_bmp);

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
				wiz_gamelist_text_out( screen_x, screen_y, drivers[i].description);
				if (aux_pos==*pos) {
					wiz_gamelist_text_out( screen_x-10, screen_y,">" );
					wiz_gamelist_text_out( screen_x-13, screen_y-1,"-" );
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
				wiz_cpu_cores=drivers[i].cores;
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
		fscanf(f,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&wiz_freq,&wiz_video_depth,&wiz_video_aspect,&wiz_video_sync,
		&wiz_frameskip,&wiz_sound,&wiz_clock_cpu,&wiz_clock_sound,&wiz_cpu_cores,&wiz_ramtweaks,&i,&wiz_cheat,&wiz_volume);
		fclose(f);
	}

	while(1)
	{
		/* Draw background image */
		load_bmp_8bpp(fb1_8bit,wizmenu_bmp);

		/* Draw the options */
		strncpy (text,game_list_description(last_game_selected),33);
		text[32]='\0';
		wiz_gamelist_text_out(x_Pos,y_Pos-10,text);

		/* (0) WIZ Clock */
		wiz_gamelist_text_out_fmt(x_Pos,y_Pos+10, "WIZ Clock     %d MHz", wiz_freq);

		/* (1) Video Depth */
		switch (wiz_video_depth)
		{
			case -1: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+20,"Video Depth   Auto"); break;
			case 8:  wiz_gamelist_text_out_fmt(x_Pos,y_Pos+20,"Video Depth   8 bit"); break;
			case 16: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+20,"Video Depth   16 bit"); break;
			default: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+20,"Video Depth   8 bit"); wiz_video_depth=8; break;
		}

		/* (2) Video Aspect */
		switch (wiz_video_aspect)
		{
			case 0: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Normal"); break;
			case 1: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Scale"); break;
			case 8: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Rotate"); break;
			case 9: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Rotate Scale"); break;
			case 24:wiz_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Diagonal Fix"); break;
			case 25:wiz_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Rotate DiagonalFix"); break;
			default:wiz_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Normal"); wiz_video_aspect=0; break; 
		}
		
		/* (3) Video Sync */
		switch (wiz_video_sync)
		{
			case 1: wiz_gamelist_text_out(x_Pos,y_Pos+40, "Video Sync    VSync"); break;
			case 0: wiz_gamelist_text_out(x_Pos,y_Pos+40, "Video Sync    Normal"); break;
			case 2: wiz_gamelist_text_out(x_Pos,y_Pos+40, "Video Sync    DblBuf"); break;
			case -1: wiz_gamelist_text_out(x_Pos,y_Pos+40,"Video Sync    OFF"); break;
		}
		
		/* (4) Frame-Skip */
		if ((wiz_video_sync==-1) && (wiz_frameskip==-1)) wiz_frameskip=0;
		if(wiz_frameskip==-1) {
			wiz_gamelist_text_out_fmt(x_Pos,y_Pos+50, "Frame-Skip    Auto");
		}
		else{
			wiz_gamelist_text_out_fmt(x_Pos,y_Pos+50,"Frame-Skip    %d",wiz_frameskip);
		}

		/* (5) Sound */
		switch(wiz_sound)
		{
			case 0: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","OFF"); break;
			case 1: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (15 KHz fast)"); break;
			case 2: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (22 KHz fast)"); break;
			case 3: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (33 KHz fast)"); break;
			case 4: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (44 KHz fast)"); break;
			case 5: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (11 KHz fast)"); break;
			case 6: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (15 KHz)"); break;
			case 7: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (22 KHz)"); break;
			case 8: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (33 KHz)"); break;
			case 9: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (44 KHz)"); break;
			case 10: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (11 KHz)"); break;
			case 11: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (15 KHz stereo)"); break;
			case 12: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (22 KHz stereo)"); break;
			case 13: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (33 KHz stereo)"); break;
			case 14: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (44 KHz stereo)"); break;
			case 15: wiz_gamelist_text_out_fmt(x_Pos,y_Pos+60,"Sound         %s","ON (11 KHz stereo)"); break;
		}

		/* (6) CPU Clock */
		wiz_gamelist_text_out_fmt(x_Pos,y_Pos+70,"CPU Clock     %d%%",wiz_clock_cpu);

		/* (7) Audio Clock */
		wiz_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Audio Clock   %d%%",wiz_clock_sound);

		/* (8) CPU cores */
		switch (wiz_cpu_cores)
		{
			case 0: wiz_gamelist_text_out(x_Pos,y_Pos+90, "CPU ASM cores None"); break;
			case 1: wiz_gamelist_text_out(x_Pos,y_Pos+90, "CPU ASM cores Cyclone"); break;
			case 2: wiz_gamelist_text_out(x_Pos,y_Pos+90, "CPU ASM cores DrZ80"); break;
			case 3: wiz_gamelist_text_out(x_Pos,y_Pos+90, "CPU ASM cores Cyclone+DrZ80"); break;
			case 4: wiz_gamelist_text_out(x_Pos,y_Pos+90, "CPU ASM cores DrZ80(snd)"); break;
			case 5: wiz_gamelist_text_out(x_Pos,y_Pos+90, "CPU ASM cores Cyclone+DrZ80(snd)"); break;
		}

		/* (9) RAM Tweaks */
		if (wiz_ramtweaks)
			wiz_gamelist_text_out(x_Pos,y_Pos+100,"RAM Tweaks    ON");
		else
			wiz_gamelist_text_out(x_Pos,y_Pos+100,"RAM Tweaks    OFF");

		/* (10) Cheats */
		if (wiz_cheat)
			wiz_gamelist_text_out(x_Pos,y_Pos+110,"Cheats        ON");
		else
			wiz_gamelist_text_out(x_Pos,y_Pos+110,"Cheats        OFF");

        /* (11) Volume */
        if (wiz_sound == 0)
        {
            wiz_gamelist_text_out(x_Pos,y_Pos+120,"Volume        Disabled");
        }
        else
        {
            switch (wiz_volume)
            {
                case 1: wiz_gamelist_text_out(x_Pos,y_Pos+120,"Volume        Quiet"); break;
                case 2: wiz_gamelist_text_out(x_Pos,y_Pos+120,"Volume        Low"); break;
                case 3: wiz_gamelist_text_out(x_Pos,y_Pos+120,"Volume        Medium"); break;
                case 4: wiz_gamelist_text_out(x_Pos,y_Pos+120,"Volume        Maximum"); break;
            }
        }
	
		wiz_gamelist_text_out(x_Pos,y_Pos+140,"Press B to confirm, X to return\0");

		/* Show currently selected item */
		wiz_gamelist_text_out(x_Pos-16,y_Pos+(selected_option*10)+10," >");

		wiz_video_flip();
		while(wiz_joystick_read(0)&0xFF0FC0) { wiz_timer_delay(150000); }
		while(!(ExKey=wiz_joystick_read(0)&0xFF0FC0)) { }
		if(ExKey & WIZ_DOWN){
			selected_option++;
			selected_option = selected_option % options_count;
		}
		else if(ExKey & WIZ_UP){
			selected_option--;
			if(selected_option<0)
				selected_option = options_count - 1;
		}
		else if(ExKey & WIZ_R || ExKey & WIZ_L)
		{
			switch(selected_option) {
			case 0:
				/* WIZ Clock */
				if(ExKey & WIZ_R){
					switch (wiz_freq) {
                        case 200: wiz_freq=300;break;
                        case 300: wiz_freq=400;break;
                        case 400: wiz_freq=500;break;
                        case 500: wiz_freq=533;break;
                        case 533: wiz_freq=650;break;
                        case 650: wiz_freq=700;break;
                        case 700: wiz_freq=750;break;
                        case 750: wiz_freq=760;break;
                        case 760: wiz_freq=770;break;
                        case 770: wiz_freq=780;break;
                        case 780: wiz_freq=790;break;
                        case 790: wiz_freq=795;break;
                        case 795: wiz_freq=800;break;
                        case 800: wiz_freq=805;break;
                        case 805: wiz_freq=810;break;
                        case 810: wiz_freq=815;break;
                        case 815: wiz_freq=820;break;
                        case 820: wiz_freq=825;break;
                        case 825: wiz_freq=830;break;
                        case 830: wiz_freq=835;break;
                        case 835: wiz_freq=840;break;
                        case 840: wiz_freq=845;break;
                        case 845: wiz_freq=850;break;
                        case 850: wiz_freq=855;break;
                        case 855: wiz_freq=860;break;
                        case 860: wiz_freq=865;break;
                        case 865: wiz_freq=870;break;
                        case 870: wiz_freq=875;break;
                        case 875: wiz_freq=880;break;
                        case 880: wiz_freq=885;break;
                        case 885: wiz_freq=890;break;
                        case 890: wiz_freq=895;break;
                        case 895: wiz_freq=900;break;
                        case 900: wiz_freq=200;break;
                        default:  wiz_freq=533;break;
					}
				} else {
					switch (wiz_freq) {
                        case 200: wiz_freq=900;break;
                        case 300: wiz_freq=200;break;
                        case 400: wiz_freq=300;break;
                        case 500: wiz_freq=400;break;
                        case 533: wiz_freq=500;break;
                        case 650: wiz_freq=533;break;
                        case 700: wiz_freq=650;break;
                        case 750: wiz_freq=700;break;
                        case 760: wiz_freq=750;break;
                        case 770: wiz_freq=760;break;
                        case 780: wiz_freq=770;break;
                        case 790: wiz_freq=780;break;
                        case 795: wiz_freq=790;break;
                        case 800: wiz_freq=795;break;
                        case 805: wiz_freq=800;break;
                        case 810: wiz_freq=805;break;
                        case 815: wiz_freq=810;break;
                        case 820: wiz_freq=815;break;
                        case 825: wiz_freq=820;break;
                        case 830: wiz_freq=825;break;
                        case 835: wiz_freq=830;break;
                        case 840: wiz_freq=835;break;
                        case 845: wiz_freq=840;break;
                        case 850: wiz_freq=845;break;
                        case 855: wiz_freq=850;break;
                        case 860: wiz_freq=855;break;
                        case 865: wiz_freq=860;break;
                        case 870: wiz_freq=865;break;
                        case 875: wiz_freq=870;break;
                        case 880: wiz_freq=875;break;
                        case 885: wiz_freq=880;break;
                        case 890: wiz_freq=885;break;
                        case 895: wiz_freq=890;break;
                        case 900: wiz_freq=895;break;
                        default:  wiz_freq=533;break;
					}
				}
				break;
			case 1:
				switch (wiz_video_depth)
				{
					case -1: wiz_video_depth=8; break;
					case 8: wiz_video_depth=16; break;
					case 16: wiz_video_depth=-1; break;
				}
				break;
			case 2:
				if(ExKey & WIZ_R)
				{
				    switch (wiz_video_aspect)
				    {
				        case 0: wiz_video_aspect=1; break;
				        case 1: wiz_video_aspect=8; break;
				        case 8: wiz_video_aspect=9; break;
				        case 9: wiz_video_aspect=24;break;
				        case 24:wiz_video_aspect=25;break;
				        case 25:wiz_video_aspect=0; break;
				    }
				}
				else
				{
				    switch (wiz_video_aspect)
				    {
				        case 0: wiz_video_aspect=25;break;
				        case 1: wiz_video_aspect=0; break;
				        case 8: wiz_video_aspect=1; break;
				        case 9: wiz_video_aspect=8; break;
				        case 24:wiz_video_aspect=9; break;
				        case 25:wiz_video_aspect=24;break;
				    }
				}
				break;
			case 3:
				wiz_video_sync=wiz_video_sync+1;
				if (wiz_video_sync>2)
					wiz_video_sync=-1;
				break;
			case 4:
				/* "Frame-Skip" */
				if(ExKey & WIZ_R)
				{
					wiz_frameskip ++;
					if (wiz_frameskip>11)
						wiz_frameskip=-1;
				}
				else
				{
					wiz_frameskip--;
					if (wiz_frameskip<-1)
						wiz_frameskip=11;
				}
				break;
			case 5:
				if(ExKey & WIZ_R)
				{
					wiz_sound ++;
					if (wiz_sound>15)
						wiz_sound=0;
				}
				else
				{
					wiz_sound--;
					if (wiz_sound<0)
						wiz_sound=15;
				}
				break;
			case 6:
				/* "CPU Clock" */
				if(ExKey & WIZ_R)
				{
					wiz_clock_cpu += 10; /* Add 10% */
					if (wiz_clock_cpu > 200) /* 200% is the max */
						wiz_clock_cpu = 200;
				}
				else
				{
					wiz_clock_cpu -= 10; /* Subtract 10% */
					if (wiz_clock_cpu < 10) /* 10% is the min */
						wiz_clock_cpu = 10;
				}
				break;
			case 7:
				/* "Audio Clock" */
				if(ExKey & WIZ_R)
				{
					wiz_clock_sound += 10; /* Add 10% */
					if (wiz_clock_sound > 200) /* 200% is the max */
						wiz_clock_sound = 200;
				}
				else{
					wiz_clock_sound -= 10; /* Subtract 10% */
					if (wiz_clock_sound < 10) /* 10% is the min */
						wiz_clock_sound = 10;
				}
				break;
			case 8:
				wiz_cpu_cores=(wiz_cpu_cores+1)%6;
				break;
			case 9:
				wiz_ramtweaks=!wiz_ramtweaks;
				break;
			case 10:
				wiz_cheat=!wiz_cheat;
				break;
            case 11:
                /* Volume */
                if(ExKey & WIZ_R)
                {
                    wiz_volume++;
                    if (wiz_volume > 4)
                        wiz_volume = 1;
                }
                else {
                    wiz_volume--;
                    if (wiz_volume < 1)
                        wiz_volume = 4;
                }
                break;
			}
		}

		if ((ExKey & WIZ_A) || (ExKey & WIZ_B) || (ExKey & WIZ_MENU)) 
		{
			/* Write game configuration */
			sprintf(text,"frontend/%s.cfg",game);
			f=fopen(text,"w");
			if (f) {
				fprintf(f,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",wiz_freq,wiz_video_depth,wiz_video_aspect,wiz_video_sync,
				wiz_frameskip,wiz_sound,wiz_clock_cpu,wiz_clock_sound,wiz_cpu_cores,wiz_ramtweaks,i,wiz_cheat,wiz_volume);
				fclose(f);
				sync();
			}

			/* Selected game will be run */
			return 1;
		}
		else if ((ExKey & WIZ_X) || (ExKey & WIZ_Y) || (ExKey & WIZ_SELECT))
		{
			/* Return To Menu */
			return 0;
		}
	}
}

static void wiz_exit(void)
{
	remove("frontend/mame.lst");
	sync();
	wiz_deinit();
	chdir("/usr/gp2x"); /*go to menu*/
      	execl("gp2xmenu", "gp2xmenu", NULL);
}

static void select_game(char *emu, char *game)
{

	unsigned long ExKey;

	/* No Selected game */
	strcpy(game,"builtinn");

	/* Clean screen */
	wiz_video_flip();

	/* Wait until user selects a game */
	while(1)
	{
		game_list_view(&last_game_selected);
		wiz_video_flip();

		if( (wiz_joystick_read(0)&0xFF0FC0)) 
			wiz_timer_delay(100000); 
		while(!(ExKey=wiz_joystick_read(0)&0xFF0FC0))
		{ 
			if ((ExKey & WIZ_L) && (ExKey & WIZ_R)) { wiz_exit(); }
		}

		if (ExKey & WIZ_UP) last_game_selected--;
		if (ExKey & WIZ_DOWN) last_game_selected++;
		if (ExKey & WIZ_L) last_game_selected-=21;
		if (ExKey & WIZ_R) last_game_selected+=21;
		if ((ExKey & WIZ_L) && (ExKey & WIZ_R)) wiz_exit();

		if ((ExKey & WIZ_A) || (ExKey & WIZ_B) || (ExKey & WIZ_MENU))
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

	/* wiz_freq */
	args[n]="-clock"; n++;
	sprintf(str[i],"%d",wiz_freq);
	args[n]=str[i]; i++; n++;

	/* wiz_video_depth */
	if (wiz_video_depth==8)
	{
		args[n]="-depth"; n++;
		args[n]="8"; n++;
	}
	if (wiz_video_depth==16)
	{
		args[n]="-depth"; n++;
		args[n]="16"; n++;
	}

	/* wiz_video_aspect */
	if ((wiz_video_aspect==1) || (wiz_video_aspect==3) || (wiz_video_aspect==5) || (wiz_video_aspect==7) ||
		(wiz_video_aspect==9) || (wiz_video_aspect==11) || (wiz_video_aspect==13) || (wiz_video_aspect==15) ||
		(wiz_video_aspect==17) || (wiz_video_aspect==19) || (wiz_video_aspect==21) || (wiz_video_aspect==23))
	{
		args[n]="-scale"; n++;
	}
	if ((wiz_video_aspect==2) || (wiz_video_aspect==3) || (wiz_video_aspect==6) || (wiz_video_aspect==7) ||
		(wiz_video_aspect==10) || (wiz_video_aspect==11) || (wiz_video_aspect==14) || (wiz_video_aspect==15) ||
		(wiz_video_aspect==18) || (wiz_video_aspect==19) || (wiz_video_aspect==22) || (wiz_video_aspect==23))
	{
		args[n]="-aspect"; n++;
	}
	if ((wiz_video_aspect==4) || (wiz_video_aspect==5) || (wiz_video_aspect==6) || (wiz_video_aspect==7) ||
		(wiz_video_aspect==12) || (wiz_video_aspect==13) || (wiz_video_aspect==14) || (wiz_video_aspect==15) ||
		(wiz_video_aspect==20) || (wiz_video_aspect==21) || (wiz_video_aspect==22) || (wiz_video_aspect==23))
	{
		args[n]="-border"; n++;
	}
  	if ((wiz_video_aspect>=8) && (wiz_video_aspect<=23))
   	{
    		args[n]="-ror"; n++;
   	}
	if ((wiz_video_aspect>=8) && (wiz_video_aspect<=15))
	{
		args[n]="-rotatecontrols"; n++;
	}
	
	/* wiz_video_sync */
	if (wiz_video_sync==1)
	{
		args[n]="-nodirty"; n++;
		args[n]="-waitvsync"; n++;
	}
	else if ((wiz_video_sync==2) || (wiz_video_aspect==1) || (wiz_video_aspect==9))
	{
		args[n]="-nodirty"; n++;
	}
	if (wiz_video_sync==-1)
	{
		args[n]="-nothrottle"; n++;
	}
	
	/* wiz_frameskip */
	if (wiz_frameskip>=0)
	{
		args[n]="-frameskip"; n++;
		sprintf(str[i],"%d",wiz_frameskip);
		args[n]=str[i]; i++; n++;
	}

	/* wiz_sound */
	if (wiz_sound==0)
	{
		args[n]="-soundcard"; n++;
		args[n]="0"; n++;
	}
	if ((wiz_sound==1) || (wiz_sound==6) || (wiz_sound==11))
	{
		args[n]="-samplerate"; n++;
		args[n]="15000"; n++;
	}
	if ((wiz_sound==2) || (wiz_sound==7) || (wiz_sound==12))
	{
		args[n]="-samplerate"; n++;
		args[n]="22050"; n++;
	}
	if ((wiz_sound==3) || (wiz_sound==8) || (wiz_sound==13))
	{
		args[n]="-samplerate"; n++;
		args[n]="32000"; n++;
	}
	if ((wiz_sound==4) || (wiz_sound==9) || (wiz_sound==14))
	{
		args[n]="-samplerate"; n++;
		args[n]="44100"; n++;
	}
	if ((wiz_sound==5) || (wiz_sound==10) || (wiz_sound==15))
	{
		args[n]="-samplerate"; n++;
		args[n]="11025"; n++;
	}
	if ((wiz_sound>=1) && (wiz_sound<=5))
	{
		args[n]="-fastsound"; n++;
	}
	if (wiz_sound>=11)
	{
		args[n]="-stereo"; n++;
	}

	/* wiz_clock_cpu */
	if (wiz_clock_cpu!=100)
	{
		args[n]="-uclock"; n++;
		sprintf(str[i],"%d",100-wiz_clock_cpu);
		args[n]=str[i]; i++; n++;
	}

	/* wiz_clock_sound */
	if (wiz_clock_cpu!=100)
	{
		args[n]="-uclocks"; n++;
		sprintf(str[i],"%d",100-wiz_clock_sound);
		args[n]=str[i]; i++; n++;
	}
	
	/* wiz_cpu_cores */
	if ((wiz_cpu_cores==1) || (wiz_cpu_cores==3) || (wiz_cpu_cores==5))
	{
		args[n]="-cyclone"; n++;
	}
	if ((wiz_cpu_cores==2) || (wiz_cpu_cores==3))
	{
		args[n]="-drz80"; n++;
	}
	if ((wiz_cpu_cores==4) || (wiz_cpu_cores==5))
	{
		args[n]="-drz80_snd"; n++;
	}

	if (wiz_ramtweaks)
	{
		args[n]="-ramtweaks"; n++;
	}
	
	if (wiz_cheat)
	{
		args[n]="-cheat"; n++;
	}

	if (wiz_video_aspect==24)
	{
		args[n]="-wiz_rotated_video"; n++;
    	args[n]="-rol"; n++;
	}
    if (wiz_video_aspect==25)
    {
		args[n]="-wiz_rotated_video"; n++;
		args[n]="-rotatecontrols"; n++;
    }

	switch (wiz_volume)
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
	
	wiz_deinit();
	execv(args[0], args); 
}


int main (int argc, char **argv)
{
	FILE *f;

	/* WIZ Initialization */
	wiz_init(8,22050,16,0);

	/* Show intro screen */
	wiz_intro_screen();

	/* Initialize list of available games */
	game_list_init(argc);
	if (game_num_avail==0)
	{
		wiz_gamelist_text_out(35, 110, "ERROR: NO AVAILABLE GAMES FOUND");
		wiz_video_flip();
		wiz_joystick_press(0);
		wiz_exit();
	}

	/* Read default configuration */
	f=fopen("frontend/mame.cfg","r");
	if (f) {
		fscanf(f,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&wiz_freq,&wiz_video_depth,&wiz_video_aspect,&wiz_video_sync,
		&wiz_frameskip,&wiz_sound,&wiz_clock_cpu,&wiz_clock_sound,&wiz_cpu_cores,&wiz_ramtweaks,&last_game_selected,&wiz_cheat,&wiz_volume);
		fclose(f);
	}
	
	/* Select Game */
	select_game(playemu,playgame); 

	/* Write default configuration */
	f=fopen("frontend/mame.cfg","w");
	if (f) {
		fprintf(f,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",wiz_freq,wiz_video_depth,wiz_video_aspect,wiz_video_sync,
		wiz_frameskip,wiz_sound,wiz_clock_cpu,wiz_clock_sound,wiz_cpu_cores,wiz_ramtweaks,last_game_selected,wiz_cheat,wiz_volume);
		fclose(f);
		sync();
	}
	
	/* Execute Game */
	execute_game (playemu,playgame);
	
	exit (0);
}
