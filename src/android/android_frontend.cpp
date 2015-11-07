#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <android/log.h>
#include "minimal.h"
#include "android_frontend_list.h"
#include "android_frontend_menu.h"
#include "android_frontend_splash.h"
#include "shared.h"

int game_num_avail=0;
static int last_game_selected=0;
char playemu[16] = "mame\0";
char playgame[16] = "builtinn\0";

int m4all_inGame=0;
int m4all_exitGame=0;

int m4all_video_aspect=0;
int m4all_video_rotate=0;
int m4all_video_sync=0;
int m4all_video_depth=8;
int m4all_frameskip=-1;
int m4all_sound = 4;
int m4all_clock_cpu= 80;
int m4all_clock_sound=80;
int m4all_cheat=0;
int m4all_buttons=2;
int m4all_waysStick = 8;
int m4all_ASMCores = 1;
int m4all_cpu_cores = 0;


extern int m4all_aspectRatio;
extern int m4all_cropVideo;
extern int m4all_fixedRes;
extern int emulated_width;
extern int emulated_height;
extern int m4all_HiSpecs;
extern int m4all_hide_LR;
extern int m4all_BplusX;
extern int m4all_landscape_buttons;

int _master_volume = 100;

extern int my_android_main (int argc, char **argv);

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
	unsigned short *p = (unsigned short *)out;
	for (y=239;y!=-1;y--) {
		for (x=0;x<320;x++) {
			//*out++=in[x+y*320];
			*p++=gp2x_palette[in[x+y*320]];
		}
	}
}

static void gp2x_intro_screen(void) {
	char name[256];
	FILE *f;
	gp2x_video_flip();
	sprintf(name,get_documents_path("skins/splash.bmp"));
	f=fopen(name,"rb");
	if (f) {
		fread(gp2xsplash_bmp,1,77878,f);
		fclose(f);
	}

	while(1)
	{
		load_bmp_8bpp(gp2x_screen8,gp2xsplash_bmp);
		gp2x_video_flip();
		int ExKey=gp2x_joystick_read(0);
		if(ExKey!=0)break;
		gp2x_timer_delay(50);
	}
	sprintf(name,get_documents_path("skins/menu.bmp"));
	f=fopen(name,"rb");
	if (f) {
		fread(gp2xmenu_bmp,1,77878,f);
		fclose(f);
	}
}

static void game_list_init(void)
{
	int i;
	DIR *d=opendir(get_documents_path("roms"));
	char game[32];
	if (d)
	{
		struct dirent *actual=readdir(d);
		while(actual)
		{
			for (i=0;i<NUMGAMES;i++)
			{
				if (_drivers[i].available==0)
				{
					sprintf(game,"%s.zip",_drivers[i].name);
					if (strcmp(actual->d_name,game)==0)
					{
						_drivers[i].available=1;
						game_num_avail++;
						break;
					}
				}
			}
			actual=readdir(d);
		}
		closedir(d);
	}
}


static void game_list_init(int argc)
{

	game_list_init();
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
		if (_drivers[i].available==1) {
			if (aux_pos>=view_pos && aux_pos<=view_pos+20) {
				gp2x_gamelist_text_out( screen_x, screen_y, _drivers[i].description);
				if (aux_pos==*pos) {
					gp2x_gamelist_text_out( screen_x-10, screen_y,">" );
					gp2x_gamelist_text_out( screen_x-13, screen_y-1,"-" );
				}
				screen_y+=8;
			}
			aux_pos++;
		}
	}

	if (game_num_avail==0)
	{
		gp2x_gamelist_text_out(35, 110, "NO AVAILABLE ROMS FOUND");
	}
#ifdef ARMV7
	gp2x_gamelist_text_out( (8*6)-8, (29*8)-6,"MAME4droid. v1.5 by D.Valdeita");
#else
	gp2x_gamelist_text_out( (8*6)-8, (29*8)-6,"MAME4droid  v1.5 by D.Valdeita");
#endif

}

static void game_list_select (int index, char *game, char *emu) {
	int i;
	int aux_pos=0;
	for (i=0;i<NUMGAMES;i++)
	{
		if (_drivers[i].available==1)
		{
			if(aux_pos==index)
			{
				strcpy(game,_drivers[i].name);
				strcpy(emu,_drivers[i].exe);
				m4all_cpu_cores=_drivers[i].cores;
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
		if (_drivers[i].available==1) {
			if(aux_pos==index) {
				return(_drivers[i].description);
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
	int y_Pos = 43;
	int options_count = 12;
	char text[256];
	FILE *f;
	int i=0;

	while(ExKey=gp2x_joystick_read(0)&0x8c0ff55){
		gp2x_video_flip();
		gp2x_timer_delay(1);
	};

	/* Read game configuration */
	sprintf(text,get_documents_path("frontend/%s_v5.cfg"),game);
	f=fopen(text,"r");
	if (f) {
		fscanf(f,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&m4all_video_aspect,&m4all_video_rotate,&m4all_video_sync,
		&m4all_frameskip,&m4all_sound,&m4all_buttons,&m4all_clock_cpu,&m4all_clock_sound,&i,&m4all_cheat,&m4all_video_depth,&m4all_waysStick,&m4all_cpu_cores);
		fclose(f);
	}

	while(1)
	{
		/* Draw background image */
		load_bmp_8bpp(gp2x_screen8,gp2xmenu_bmp);

		/* Draw the options */
		gp2x_gamelist_text_out(x_Pos,y_Pos,"Selected Game:\0");
		strncpy (text,game_list_description(last_game_selected),33);
		text[32]='\0';
		gp2x_gamelist_text_out(x_Pos,y_Pos+10,text);

		/* (1) Video Aspect */
		switch (m4all_video_aspect)
		{
			case 0: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Original"); break;
			/*
			case 1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Ratio Not Kept"); break;
			*/
			case 1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Cropping 4/3"); break;
			case 2: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Cropping 3/4"); break;

			case 3: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Fixed 320x240"); break;
			case 4: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Fixed 240x320"); break;
			case 5: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Fixed 640x480"); break;
			case 6: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+30,"Video Aspect  Fixed 480x640"); break;
		}

		/* (2) Video Rotation */
		switch (m4all_video_rotate)
		{
			case 0: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+40,"Video Rotate  No"); break;
			case 1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+40,"Video Rotate  Yes"); break;
			case 2: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+40,"Video Rotate  TATE"); break;
		}
		
		/* (3) Video Depth */
		switch (m4all_video_depth)
		{
			case -1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+50,"Video Depth   Auto"); break;
			case 8:  gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+50,"Video Depth   8 bit"); break;
			case 16: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+50,"Video Depth   16 bit"); break;
		}

		/* (4) Video Sync */
		switch (m4all_video_sync)
		{
			case 0: gp2x_gamelist_text_out(x_Pos,y_Pos+60, "Video Sync    Normal"); break;
			case 1: gp2x_gamelist_text_out(x_Pos,y_Pos+60, "Video Sync    DblBuf"); break;
			case -1: gp2x_gamelist_text_out(x_Pos,y_Pos+60,"Video Sync    OFF"); break;
		}
		
		/* (5) Frame-Skip */
		if(m4all_frameskip==-1) {
			gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70, "Frame-Skip    Auto");
		}
		else{
			gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70,"Frame-Skip    %d",m4all_frameskip);
		}

		/* (6) Sound */
		switch(m4all_sound)
		{
			case 0: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Sound         %s","OFF"); break;

			case 1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Sound         %s","ON (11 KHz fast)"); break;
			case 2: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Sound         %s","ON (22 KHz fast)"); break;
			case 3: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Sound         %s","ON (33 KHz fast)"); break;
			case 4: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Sound         %s","ON (44 KHz fast)"); break;

			case 5: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Sound         %s","ON (11 KHz)"); break;
			case 6: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Sound         %s","ON (22 KHz)"); break;
			case 7: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Sound         %s","ON (33 KHz)"); break;
			case 8: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Sound         %s","ON (44 KHz)"); break;

			case 9: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Sound         %s","ON (11 KHz stereo)"); break;
			case 10: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Sound         %s","ON (22 KHz stereo)"); break;
			case 11: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Sound         %s","ON (33 KHz stereo)"); break;
			case 12: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+80,"Sound         %s","ON (44 KHz stereo)"); break;

		}

		/* (7) Landscape Num Buttons */
		switch (m4all_waysStick)
		{
			case 8: gp2x_gamelist_text_out(x_Pos,y_Pos+90, "Stick         8-way"); break;
			case 4: gp2x_gamelist_text_out(x_Pos,y_Pos+90, "Stick         4-way"); break;
			case 2: gp2x_gamelist_text_out(x_Pos,y_Pos+90, "Stick         2-way"); break;
		}

		/* (8) Landscape Num Buttons */
		switch (m4all_buttons)
		{
		    case 0: gp2x_gamelist_text_out(x_Pos,y_Pos+100, "Num Buttons   0 Button"); break;
		    case 1: gp2x_gamelist_text_out(x_Pos,y_Pos+100, "Num Buttons   1 Button"); break;
			case 2: gp2x_gamelist_text_out(x_Pos,y_Pos+100, "Num Buttons   2 Buttons"); break;
			case 3: gp2x_gamelist_text_out(x_Pos,y_Pos+100, "Num Buttons   3 Buttons (A=B+X)"); break;
			case 4: gp2x_gamelist_text_out(x_Pos,y_Pos+100, "Num Buttons   3 Buttons"); break;
			case 5: gp2x_gamelist_text_out(x_Pos,y_Pos+100, "Num Buttons   4 Buttons"); break;
			case 6: gp2x_gamelist_text_out(x_Pos,y_Pos+100, "Num Buttons   All Buttons"); break;
		}

		/* (9) CPU Clock */
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+110,"CPU Clock     %d%%",m4all_clock_cpu);

		/* (10) Audio Clock */
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+120,"Audio Clock   %d%%",m4all_clock_sound);

		/* (11) CPU ASM Cores */

		if(m4all_ASMCores)
		{
			switch (m4all_cpu_cores)
			{
				case 0: gp2x_gamelist_text_out(x_Pos,y_Pos+130, "CPU ASM cores None"); break;
				case 1: gp2x_gamelist_text_out(x_Pos,y_Pos+130, "CPU ASM cores Cyclone"); break;
				case 2: gp2x_gamelist_text_out(x_Pos,y_Pos+130, "CPU ASM cores DrZ80"); break;
				case 3: gp2x_gamelist_text_out(x_Pos,y_Pos+130, "CPU ASM cores Cyclone+DrZ80"); break;
				case 4: gp2x_gamelist_text_out(x_Pos,y_Pos+130, "CPU ASM cores DrZ80(snd)"); break;
				case 5: gp2x_gamelist_text_out(x_Pos,y_Pos+130, "CPU ASM cores Cyclone+DrZ80(snd)"); break;
				case 6: gp2x_gamelist_text_out(x_Pos,y_Pos+130, "CPU ASM cores All"); break;
			}
		}
		else
		{
			gp2x_gamelist_text_out(x_Pos,y_Pos+130, "CPU ASM cores None");
		}

		/* (12) Cheats */
		if (m4all_cheat)
			gp2x_gamelist_text_out(x_Pos,y_Pos+140,"Cheats        ON");
		else
			gp2x_gamelist_text_out(x_Pos,y_Pos+140,"Cheats        OFF");

	
		gp2x_gamelist_text_out(x_Pos,y_Pos+160,"Press B to confirm, X to return\0");

		/* Show currently selected item */
		gp2x_gamelist_text_out(x_Pos-16,y_Pos+(selected_option*10)+30," >");

		gp2x_video_flip();

		gp2x_timer_delay(150);
		ExKey=gp2x_joystick_read(0)&0x8c0ff55;

		if(ExKey & GP2X_DOWN){
			selected_option++;
			selected_option = selected_option % options_count;
		}
		else if(ExKey & GP2X_UP){
			selected_option--;
			if(selected_option<0)
				selected_option = options_count - 1;
		}
		else if(ExKey & GP2X_R || ExKey & GP2X_L || ExKey & GP2X_RIGHT || ExKey & GP2X_LEFT)
		{
			switch(selected_option) {
			case 0:
				if((ExKey & GP2X_R) || (ExKey & GP2X_RIGHT))
				{
					m4all_video_aspect++;
					if (m4all_video_aspect>6)
						m4all_video_aspect=0;
				}
				else
				{

					m4all_video_aspect--;
					if (m4all_video_aspect<0)
						m4all_video_aspect=6;
				}
				break;
			case 1:
				if((ExKey & GP2X_R) || (ExKey & GP2X_RIGHT))
				{
					m4all_video_rotate++;
					if (m4all_video_rotate>2)
						m4all_video_rotate=0;
				}
				else
				{
					m4all_video_rotate--;
					if (m4all_video_rotate<0)
						m4all_video_rotate=2;
				}
				break;
			case 2:
				switch (m4all_video_depth)
				{
					case -1: m4all_video_depth=8; break;
					case 8: m4all_video_depth=16; break;
					case 16: m4all_video_depth=-1; break;
				}
				break;
			case 3:
				m4all_video_sync=m4all_video_sync+1;
				if (m4all_video_sync>1)
					m4all_video_sync=-1;
				break;
			case 4:
				/* "Frame-Skip" */
				if(ExKey & GP2X_R || ExKey & GP2X_RIGHT )
				{
					m4all_frameskip ++;
					if (m4all_frameskip>11)
						m4all_frameskip=-1;
				}
				else
				{
					m4all_frameskip--;
					if (m4all_frameskip<-1)
						m4all_frameskip=11;
				}
				break;
			case 5:
				if(ExKey & GP2X_R || ExKey & GP2X_RIGHT)
				{
					m4all_sound ++;
					if (m4all_sound>12)
						m4all_sound=0;
				}
				else
				{
					m4all_sound--;
					if (m4all_sound<0)
						m4all_sound=12;
				}
				break;
			case 6:
				switch (m4all_waysStick)
				{
					case 8: m4all_waysStick=4; break;
					case 4: m4all_waysStick=2; break;
					case 2: m4all_waysStick=8; break;
				}
				break;
			case 7:
				if(ExKey & GP2X_R || ExKey & GP2X_RIGHT)
				{
					m4all_buttons ++;
					if (m4all_buttons>6)
						m4all_buttons=0;
				}
				else
				{
					m4all_buttons--;
					if (m4all_buttons<0)
						m4all_buttons=6;
				}
				break;
			case 8:
				/* "CPU Clock" */
				if(ExKey & GP2X_R || ExKey & GP2X_RIGHT)
				{
					m4all_clock_cpu += 10; /* Add 10% */
					if (m4all_clock_cpu > 200) /* 200% is the max */
						m4all_clock_cpu = 200;
				}
				else
				{
					m4all_clock_cpu -= 10; /* Subtract 10% */
					if (m4all_clock_cpu < 10) /* 10% is the min */
						m4all_clock_cpu = 10;
				}
				break;
			case 9:
				/* "Audio Clock" */
				if(ExKey & GP2X_R || ExKey & GP2X_RIGHT)
				{
					m4all_clock_sound += 10; /* Add 10% */
					if (m4all_clock_sound > 200) /* 200% is the max */
						m4all_clock_sound = 200;
				}
				else{
					m4all_clock_sound -= 10; /* Subtract 10% */
					if (m4all_clock_sound < 10) /* 10% is the min */
						m4all_clock_sound = 10;
				}
				break;
			case 10:
				m4all_cpu_cores=(m4all_cpu_cores+1)%7;
				break;
			case 11:
				m4all_cheat=!m4all_cheat;
				break;
			}
		}

		if ((ExKey & GP2X_A) || (ExKey & GP2X_B) || (ExKey & GP2X_PUSH) || (ExKey & GP2X_START))
		{
			/* Write game configuration */
			sprintf(text,get_documents_path("frontend/%s_v5.cfg"),game);
			f=fopen(text,"w");
			if (f) {
				fprintf(f,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",m4all_video_aspect,m4all_video_rotate,m4all_video_sync,
				m4all_frameskip,m4all_sound,m4all_buttons,m4all_clock_cpu,m4all_clock_sound,i,m4all_cheat,m4all_video_depth,m4all_waysStick,m4all_cpu_cores);
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
}

static void select_game(char *emu, char *game)
{

	unsigned long ExKey;

	/* No Selected game */
	strcpy(game,"builtinn");

	/* Clean screen */
	gp2x_video_flip();


	while(ExKey=gp2x_joystick_read(0)&0x8c0ff55){
		gp2x_video_flip();
		gp2x_timer_delay(1);
	};

	/* Wait until user selects a game */
	while(1)
	{
		game_list_view(&last_game_selected);
		gp2x_video_flip();

		gp2x_timer_delay(100);
       	ExKey=gp2x_joystick_read(0);

		if (ExKey & GP2X_UP) last_game_selected--;
		else if (ExKey & GP2X_DOWN) last_game_selected++;
		else if ((ExKey & GP2X_L) || ExKey & GP2X_LEFT) last_game_selected-=21;
		else if ((ExKey & GP2X_R) || ExKey & GP2X_RIGHT) last_game_selected+=21;
		//if ((ExKey & GP2X_L) && (ExKey & GP2X_R)) gp2x_exit();

		if (((ExKey & GP2X_A) || (ExKey & GP2X_B) || (ExKey & GP2X_PUSH) || (ExKey & GP2X_START)) && game_num_avail!=0)
		{
			/* Select the game */
			game_list_select(last_game_selected, game, emu);

			/* Emulation Options */
			//defaults!

			m4all_video_aspect=0;
			m4all_video_rotate=0;
			m4all_video_sync=0;
			m4all_frameskip=-1;
			m4all_cheat=0;
            m4all_waysStick = 8;

			m4all_sound = 4;
			m4all_video_depth=16;

			if(m4all_HiSpecs)
			{
				m4all_clock_cpu= 100;
				m4all_clock_sound= 100;
				m4all_buttons=2;
				m4all_sound=12;
			}
			else
			{
				m4all_clock_cpu= 80;
				m4all_clock_sound= 80;
				m4all_buttons=2;
			}

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
/*
	args[n]="-depth"; n++;
	if(!safe_render_path)
		args[n]="8";
	else
		args[n]="16";
	n++;
	*/

	/* gp2x_video_depth */
	if (m4all_video_depth==8)
	{
		args[n]="-depth"; n++;
		args[n]="8"; n++;
	}
	if (m4all_video_depth==16)
	{
		args[n]="-depth"; n++;
		args[n]="16"; n++;
	}

	/* iOS_video_aspect */
	m4all_aspectRatio = m4all_cropVideo = m4all_fixedRes = 0;
    if (m4all_video_aspect==0)
	{
    	m4all_aspectRatio = 1;
    }else if(m4all_video_aspect==1){
		m4all_cropVideo = 1;
    }else if(m4all_video_aspect==2){
		m4all_cropVideo = 2;
    }else if(m4all_video_aspect==3){
		m4all_fixedRes = 1;
		//printf("fixed %d,%d,%d\n",iOS_aspectRatio,iOS_cropVideo,iOS_320x240);
    }else if(m4all_video_aspect==4){
		m4all_fixedRes = 2;
    }else if(m4all_video_aspect==5){
		m4all_fixedRes = 3;
    }else if(m4all_video_aspect==6){
		m4all_fixedRes = 4;
    }

	/* iOS_video_rotate */
	if ((m4all_video_rotate>=1) && (m4all_video_rotate<=2))
	{
		args[n]="-ror"; n++;
	}

	if ((m4all_video_rotate>=2) && (m4all_video_rotate<=2))
	{
		args[n]="-rotatecontrols"; n++;
	}
	
	/* iOS_video_sync */
    if (m4all_video_sync==1)
	{
		args[n]="-nodirty"; n++;
	}
	else if (m4all_video_sync==-1)
	{
		args[n]="-nothrottle"; n++;
	}
	
	/* iOS_frameskip */
	if (m4all_frameskip>=0)
	{
		args[n]="-frameskip"; n++;
		sprintf(str[i],"%d",m4all_frameskip);
		args[n]=str[i]; i++; n++;
	}

	/* iOS_sound */
	if (m4all_sound==0)
	{
		args[n]="-soundcard"; n++;
		args[n]="0"; n++;
	}
	if ((m4all_sound==1) || (m4all_sound==5) || (m4all_sound==9))
	{
		args[n]="-samplerate"; n++;
		args[n]="11025"; n++;
	}
	if ((m4all_sound==2) || (m4all_sound==6) || (m4all_sound==10))
	{
		args[n]="-samplerate"; n++;
		args[n]="22050"; n++;
	}
	if ((m4all_sound==3) || (m4all_sound==7) || (m4all_sound==11))
	{
		args[n]="-samplerate"; n++;
		args[n]="32000"; n++;
	}
	if ((m4all_sound==4) || (m4all_sound==8) || (m4all_sound==12))
	{
		args[n]="-samplerate"; n++;
		args[n]="44100"; n++;
	}

	if ((m4all_sound>=1) && (m4all_sound<=4))
	{
		args[n]="-fastsound"; n++;
	}
	if (m4all_sound>=9)
	{
		args[n]="-stereo"; n++;
	}

	/* iOS_clock_cpu */
	if (m4all_clock_cpu!=100)
	{
		args[n]="-uclock"; n++;
		sprintf(str[i],"%d",100-m4all_clock_cpu);
		args[n]=str[i]; i++; n++;
	}

	/* iOS_clock_sound */
	if (m4all_clock_cpu!=100)
	{
		args[n]="-uclocks"; n++;
		sprintf(str[i],"%d",100-m4all_clock_sound);
		args[n]=str[i]; i++; n++;
	}

	__android_log_print(ANDROID_LOG_DEBUG, "libMAME4all.so", "ASM CORES %d %d\n",m4all_ASMCores,m4all_cpu_cores);

	/* cpu_cores */
	if(m4all_ASMCores)
	{
		if ((m4all_cpu_cores==1) || (m4all_cpu_cores==3) || (m4all_cpu_cores==5) || (m4all_cpu_cores==6))
		{
			args[n]="-cyclone"; n++;
		}

		if ((m4all_cpu_cores==2) || (m4all_cpu_cores==3) || (m4all_cpu_cores==6))
		{
			args[n]="-drz80"; n++;
		}

		if ((m4all_cpu_cores==4) || (m4all_cpu_cores==5) || (m4all_cpu_cores==6))
		{
			args[n]="-drz80_snd"; n++;
		}
	}

	if (m4all_cheat)
	{
		args[n]="-cheat"; n++;
	}

	if (0)
	{
		args[n]="-romdir"; n++;
		sprintf(str[i],"%s",get_documents_path("roms"));
		args[n]=str[i]; i++; n++;
	}

	args[n]=NULL;
	
	for (i=0; i<n; i++)
	{
		printf("%s ",args[i]);
		__android_log_print(ANDROID_LOG_DEBUG, "libMAME4all.so", "arg: %s\n",args[i]);
	}
	printf("\n");
	
	m4all_inGame = 1;
	m4all_exitGame=0;
	m4all_hide_LR = m4all_buttons!=6;
	m4all_BplusX = m4all_buttons==3;
	m4all_landscape_buttons = m4all_buttons <= 3 ? m4all_buttons : (m4all_buttons - 1) ;
	//gp2x_set_video_mode(16,320,240);

	my_android_main(n, args);

	if(m4all_HiSpecs)
		m4all_buttons=2;
	else
		m4all_buttons=2;

	m4all_hide_LR = 0;
	m4all_BplusX = 0;

	m4all_exitGame=0;
	m4all_inGame = 0;
	m4all_landscape_buttons = 2;
	emulated_width = 320;
	emulated_height = 240;
	gp2x_set_video_mode(16,320,240);

}


extern "C" int android_main  (int argc, char **argv)
{
	FILE *f;

	__android_log_print(ANDROID_LOG_DEBUG, "libMAME4all.so", "init iOS frontend");
	//printf("init iOS frontend\n");
/*
	mkdir(get_documents_path("iOS"), 0755);
	mkdir(get_documents_path("cfg"), 0755);
	mkdir(get_documents_path("hi"), 0755);
*/
	__android_log_print(ANDROID_LOG_DEBUG, "libMAME4all.so", "creados directorios");

	/* GP2X Initialization */
	gp2x_init(1000,8,22050,16,0,60);

	//hack: por defecto lentos van a 11000
	m4all_sound = 4;
	m4all_video_depth = 16;

	if(m4all_HiSpecs)
	{
		m4all_clock_cpu= 100;
		m4all_clock_sound= 100;
		m4all_buttons=2;
		m4all_sound=12;
	}

	/* Show intro screen */
	gp2x_intro_screen();

	/* Initialize list of available games */
	game_list_init(argc);

	/*
	if (game_num_avail==0)
	{   while(true){
		gp2x_gamelist_text_out(35, 110, "ERROR: NO AVAILABLE GAMES FOUND");
		gp2x_video_flip();
		gp2x_joystick_press(0);
		//gp2x_exit();
		sleep(1);}
		exit(0);
	}
    */

	while(true)
	{
	/* Read default configuration */
	f=fopen(get_documents_path("frontend/mame_v2.cfg"),"r");
	if (f) {
		fscanf(f,"%d",&last_game_selected);
		fclose(f);
	}
	
	/* Select Game */
	select_game(playemu,playgame); 

	/* Write default configuration */
	f=fopen(get_documents_path("frontend/mame_v2.cfg"),"w");
	if (f) {
		fprintf(f,"%d",last_game_selected);
		fclose(f);
		sync();
	}
	
	/* Execute Game */
	execute_game (playemu,playgame);


	}
	exit (0);
}
