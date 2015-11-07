#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "minimal.h"
#include "iOS_frontend_list.h"
#include "iOS_frontend_menu.h"
#include "iOS_frontend_splash.h"
#include "shared.h"

int game_num_avail=0;
static int last_game_selected=0;
char playemu[16] = "mame\0";
char playgame[16] = "builtinn\0";

int iOS_inGame=0;
int iOS_exitGame=0;

int iOS_video_aspect=0;
int iOS_video_rotate=0;
int iOS_video_sync=0;
int iOS_video_depth=8;
int iOS_frameskip=-1;
int iOS_sound = 12;
int iOS_clock_cpu= 100;
int iOS_clock_sound=100;
int iOS_cheat=0;
int iOS_buttons=2;
int iOS_waysStick = 8;


extern int iOS_aspectRatio;
extern int iOS_cropVideo;
extern int iOS_fixedRes;
extern int emulated_width;
extern int emulated_height;

extern int safe_render_path;
extern int isIpad;
extern int iOS_hide_LR;
extern int iOS_BplusX;
extern int iOS_landscape_buttons;

extern int global_sound;

extern int global_manufacturer;
extern int global_category;
extern int global_filter;
extern int global_clones;
extern int global_year;

static int local_manufacturer;
static int local_category;
static int local_clones;
static int local_filter;
static int local_year;

int _master_volume = 100;

extern int iphone_main (int argc, char **argv);

#define MAXFAVS 1000
static char favarray[MAXFAVS][9];

static int game_list_array[NUMGAMES];
extern int game_list_num;
int local_game_list_num;

char manufact_array[][16] = { "All", "Atari", "Capcom", "Cinematronics", "Data East", "Exidy", "Gremlin", "Irem", "Jaleco", "Leland", "Konami", "Midway", "Namco", "NeoGeo", "Nichibutsu", "Nintendo", "Other", "Sega", "SNK", "Stern", "Taito", "Technos", "Tecmo", "Toaplan", "Universal", "Video System", "Williams", "" };

char category_array[][14] = { "All", "Ball & Paddle", "Breakout", "Casino", "Climbing", "Driving", "Fighter", "Maze", 
    "Mini-Games", "Misc", "Multiplay", "Pinball", "Platform", "Puzzle", "Quiz", "Shooter", 
    "Sports", "Tabletop", "Wrestling" };

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
	char name[1024];
	FILE *f;
	gp2x_video_flip();
	sprintf(name,get_resource_path("skins/iOSsplash.bmp"));
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
        usleep(50000);
	}
	sprintf(name,get_resource_path("skins/iOSmenu.bmp"));
	f=fopen(name,"rb");
	if (f) {
		fread(gp2xmenu_bmp,1,77878,f);
		fclose(f);
	}
}

static void favorites_read(void)
{
    //SQ: Read the favorites list from the favorites.ini file
    FILE *file;
    char filename[1024];
    int counter=0;
    int startread=0;
    
    favarray[0][0] = '\0';
    
    sprintf(filename,get_documents_path("folders/Favorites.ini"));
    
    file = fopen (filename, "r");
    if ( file != NULL )
    {
        char line[256]; /* or other suitable maximum line size */

        while ( fgets(line, sizeof line, file) != NULL ) /* read a line */
        {
            if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';
            if (line[strlen(line) - 1] == '\r') line[strlen(line) - 1] = '\0';

            if (line[0] == '[' || line[0] == '\0' || strlen(line) > 8) continue;

            //Everything checks out so stick the line in the array
            strcpy(favarray[counter++], line);
			if(counter == MAXFAVS-2) break;
        }
        fclose ( file );
		favarray[counter][0] = '\0';
    }
}

static void favorites_remove(char *game) 
{
    //SQ: Scan through the favorites file and remove
    //the requested line, creating a new file.
    FILE *file, *file2;
    char filename[1024], filename2[1024];
    int counter=0;
    int startread=0;

    sprintf(filename,get_documents_path("folders/Favorites.ini"));
    sprintf(filename2, "%s.new", filename);
    
    file = fopen (filename, "r");
    file2 = fopen (filename2, "w");
    if ( file != NULL && file2 != NULL) {
        char line[256]; 
        char line2[256];
        
        while ( fgets(line, sizeof line, file) != NULL ) { /* read a line */
            strcpy(line2, line);
            
            if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';
            if (line[strlen(line) - 1] == '\r') line[strlen(line) - 1] = '\0';
            
            if (line[0] != '[' && line[0] != '\0' && strlen(line) <= 8) {
                if (strcmp(line, game) == 0) {
                    continue;
                }                     
            }
            fputs(line2, file2); 
            
        }
        fclose (file);
        fclose (file2);
        
        //Move the new file over the old one.
        rename(filename2, filename);
    }
    
    //SQ:All done so re-read the favorites array
    favorites_read();
} 

static void favorites_add(char *game) 
{
    //SQ:Add the game to the favorites file
    FILE *file;
    char filename[1024];
    
    //SQ:Make sure the directory exists before creating a new file
    mkdir(get_documents_path("folders"), 0777);
    
    sprintf(filename,get_documents_path("folders/Favorites.ini"));
    file = fopen(filename, "a");
    if (file != NULL) {
        fputs(game, file);
        fputc('\n', file);
        fclose(file);
    }
    
    //SQ:All done so re-read the favorites array
    favorites_read();
}
    
    
static void game_list_init(void)
{
	int i, j;
    int skipflag=0;
    int counter;
	int check_neogeo=0, check_other=0;

	//SQ: read the favorites
    favorites_read();	

    //SQ: Read the directory rom listing into memory to cache it for later filtering
	//We only cache the game index not the actual rom name
    if(!game_list_num) {
        DIR *d=opendir(get_documents_path("."));
        game_num_avail=0;
        if (d)
        {
			char tempstr[50];
            struct dirent *actual=readdir(d);
            while(actual)
            {
                if(strlen(actual->d_name) > 5) {
				    strcpy(tempstr, actual->d_name);
                    tempstr[(strlen(tempstr) - 4)] = '\0';	//remove .zip to compare in loop
                    for (i=0;i<NUMGAMES;i++) {
                        if (strcmp(tempstr, _drivers[i].name)==0) {
    						game_list_array[game_list_num++] = i;
                            break;
                        }
                    }
				}
                actual=readdir(d);
			}
            closedir(d);
		}
        local_game_list_num=game_list_num;
	}

    //SQ: Reset list, may happen if filter changes
    for (i=0;i<NUMGAMES;i++) {
        _drivers[i].available=0;
    }

	//SQ: Set these flags to save many string comparisons in loop below
	if (strcmp(manufact_array[global_manufacturer], "NeoGeo") == 0) check_neogeo=1;
    if (strcmp(manufact_array[global_manufacturer], "Other") == 0) check_other=1;
    
    game_num_avail=0;
    
	//SQ:game_list_array contains a list of all the ROMS in the /roms directory
    for(j=0;j<game_list_num;j++) {
        for (i=0;i<NUMGAMES;i++)
        {                
            if (!_drivers[i].available)
            {
                if (game_list_array[j] == i)     //Found a matching game
                {
                    skipflag=0;

                    //SQ:Are clones to be displayed
                    if (!global_clones && !skipflag) {
                        if (_drivers[i].clone) skipflag=1;
                    }

                    //SQ:If manufacturers is filtered, only display the relevant games
                    if (global_manufacturer && !skipflag) {
                        skipflag=1;

						//SQ:Check for Neogeo games if Neogeo selected, special case as its driver specific
                		if (check_neogeo) {
                        	if (strcmp(_drivers[i].exe, "neomame") == 0) skipflag=0;   
						} 
                		else if (check_other) {	//SQ:find all manufacturers that aren't in the list
							int x=0, mfound=0;
							while(true) {
							    if (strstr(_drivers[i].manufacturer, manufact_array[x])) {
									mfound=1;
									break;
								}
								x++;
								if(manufact_array[x][0] == '\0') break;
							}
							if(!mfound) skipflag=0;
						}
                        else if (strstr(_drivers[i].manufacturer, manufact_array[global_manufacturer])) skipflag=0;
                    }        

                    //SQ:Check category filter
                    if (global_category && !skipflag) {
                        if (!strstr(_drivers[i].category, category_array[global_category])) skipflag=1;
                    }        
                    
					//SQ:Filter by year
					if(global_year && !skipflag) {
						if((global_year + 1974) != _drivers[i].year) skipflag=1;
					}
                    
                    //SQ:Show only favorites
                    if(global_filter == 1 && !skipflag) {	
                        skipflag=1;
                        counter=0;
                        while(true) {
                            if (favarray[counter][0] == '\0') break;	//Null is the array terminator
                            if (strcasecmp(favarray[counter], _drivers[i].name) == 0) {
                                skipflag=0;
                                break;
                            }
                            counter++;
                        }
                    }

                    //SQ:Everything matches so add to list
                    if(!skipflag) {                       
                        _drivers[i].available=1;
                        game_num_avail++;
                    }
                    break;
                }
            }
        }
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
    char tempstr[255];

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
                //Check if the game is a favorite
                int foundfav=0;
                int counter=0;
                while(true) {
                    if (favarray[counter][0] == '\0') break;	//Null is the array terminator
                    if (strcasecmp(favarray[counter], _drivers[i].name) == 0) {
                        foundfav=1;
                        break;
                    }
                    counter++;
                }           
                if(foundfav) {
                    gp2x_gamelist_text_out_color( screen_x, screen_y, _drivers[i].description, 3); //light blue
                } else {
                    gp2x_gamelist_text_out( screen_x, screen_y, _drivers[i].description);  //white
                }
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
		gp2x_gamelist_text_out(35, 110, "NO AVAILABLE GAMES FOUND");
	}
    
    //Print the filter on the bottom line if it is set
    if(global_manufacturer || global_category || global_year) {
        char tempstr2[5];
        strcpy(tempstr, "Filter:");
        if (global_year) {
            sprintf(tempstr2, "%d", global_year+1974);
            strcat(tempstr, tempstr2);
            strcat(tempstr, "/");
        }
        if(global_category) {
            strcat(tempstr, category_array[global_category]);
            strcat(tempstr, "/");
        }
        if(global_manufacturer) {
            strcat(tempstr, manufact_array[global_manufacturer]);
            strcat(tempstr, "/");
        }
        tempstr[strlen(tempstr)-1] = '\0';
        gp2x_gamelist_text_out( screen_x-20, (29*8)-6,tempstr);
    } else {
        gp2x_gamelist_text_out( (8*6)-18, (29*8)-6,"iMAME4all v1.11.0b by D.Valdeita");
    }
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


static char *game_list_manufacturer (int index)
{
	int i;
	int aux_pos=0;
	for (i=0;i<NUMGAMES;i++) {
		if (_drivers[i].available==1) {
			if(aux_pos==index) {
                char tempstr[255];
                sprintf(tempstr, "%d %s", _drivers[i].year, _drivers[i].manufacturer);
				return(tempstr);
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
	int y_Pos = 48;
	int options_count = 11;
	char text[256];
	FILE *f;
	int i=0;

	if(!safe_render_path)
	  while(ExKey=gp2x_joystick_read(0)&0x8c0ff55){usleep(1000);};

	/* Read game configuration */
	sprintf(text,get_documents_path("iOS/%s_v4.cfg"),game);
	f=fopen(text,"r");
	if (f) {
		fscanf(f,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&iOS_video_aspect,&iOS_video_rotate,&iOS_video_sync,
		&iOS_frameskip,&iOS_sound,&iOS_buttons,&iOS_clock_cpu,&iOS_clock_sound,&i,&iOS_cheat,&iOS_video_depth,&iOS_waysStick);
		fclose(f);
	}

	if(!safe_render_path && iOS_video_aspect!=3 && iOS_video_aspect!=4)
	{
	   iOS_video_aspect=3;
	}

	while(1)
	{
		/* Draw background image */
		load_bmp_8bpp(gp2x_screen8,gp2xmenu_bmp);

		/* Draw the options */
		strncpy (text,game_list_description(last_game_selected),33);
		text[32]='\0';
		gp2x_gamelist_text_out(x_Pos,y_Pos,text);
        
        strncpy (text,game_list_manufacturer(last_game_selected),33);
        text[32]='\0';
        gp2x_gamelist_text_out(x_Pos,y_Pos+10,text);
        

		/* (1) Video Aspect */
		switch (iOS_video_aspect)
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
		switch (iOS_video_rotate)
		{
			case 0: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+40,"Video Rotate  No"); break;
			case 1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+40,"Video Rotate  Yes"); break;
			case 2: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+40,"Video Rotate  TATE"); break;
		}
		
		/* (3) Video Depth */
		switch (iOS_video_depth)
		{
			case -1: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+50,"Video Depth   Auto"); break;
			case 8:  gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+50,"Video Depth   8 bit"); break;
			case 16: gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+50,"Video Depth   16 bit"); break;
		}

		/* (4) Video Sync */
		switch (iOS_video_sync)
		{
			case 0: gp2x_gamelist_text_out(x_Pos,y_Pos+60, "Video Sync    Normal"); break;
			case 1: gp2x_gamelist_text_out(x_Pos,y_Pos+60, "Video Sync    DblBuf"); break;
			case -1: gp2x_gamelist_text_out(x_Pos,y_Pos+60,"Video Sync    OFF"); break;
		}
		
		/* (5) Frame-Skip */
		if(iOS_frameskip==-1) {
			gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70, "Frame-Skip    Auto");
		}
		else{
			gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+70,"Frame-Skip    %d",iOS_frameskip);
		}

		/* (6) Sound */
		switch(iOS_sound)
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
		switch (iOS_waysStick)
		{
			case 8: gp2x_gamelist_text_out(x_Pos,y_Pos+90, "Stick         8-way"); break;
			case 4: gp2x_gamelist_text_out(x_Pos,y_Pos+90, "Stick         4-way"); break;
			case 2: gp2x_gamelist_text_out(x_Pos,y_Pos+90, "Stick         2-way"); break;
		}

		/* (8) Landscape Num Buttons */
		switch (iOS_buttons)
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
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+110,"CPU Clock     %d%%",iOS_clock_cpu);

		/* (10) Audio Clock */
		gp2x_gamelist_text_out_fmt(x_Pos,y_Pos+120,"Audio Clock   %d%%",iOS_clock_sound);


		/* (11) Cheats */
		if (iOS_cheat)
			gp2x_gamelist_text_out(x_Pos,y_Pos+130,"Cheats        ON");
		else
			gp2x_gamelist_text_out(x_Pos,y_Pos+130,"Cheats        OFF");
	
		gp2x_gamelist_text_out(x_Pos,y_Pos+150,"Press B to confirm, X to return\0");

		/* Show currently selected item */
		gp2x_gamelist_text_out(x_Pos-16,y_Pos+(selected_option*10)+30," >");

		gp2x_video_flip();

		if(safe_render_path)
		{
		   while(gp2x_joystick_read(0)&0x8c0ff55) { 
               usleep(150000);
           }
		   while(!(ExKey=gp2x_joystick_read(0)&0x8c0ff55)) { usleep(1000); }
		}
		else
		{
           usleep(150000);
		   ExKey=gp2x_joystick_read(0)&0x8c0ff55;
		}

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
				/*
				if(!safe_render_path)
				{
					iOS_video_aspect=3;
					break;
				}
				*/
				if((ExKey & GP2X_R) || (ExKey & GP2X_RIGHT))
				{

					if(safe_render_path)
					{
						iOS_video_aspect++;
						if (iOS_video_aspect>6)
							iOS_video_aspect=0;
					}
					else
					{
						iOS_video_aspect++;
						if (iOS_video_aspect>4)
							iOS_video_aspect=3;
					}


				}
				else
				{
					if(safe_render_path)
					{
						iOS_video_aspect--;
						if (iOS_video_aspect<0)
							iOS_video_aspect=6;
					}
					else
					{
						iOS_video_aspect--;
						if (iOS_video_aspect<3)
							iOS_video_aspect=4;
					}
				}
				break;
			case 1:
				if((ExKey & GP2X_R) || (ExKey & GP2X_RIGHT))
				{
					iOS_video_rotate++;
					if (iOS_video_rotate>2)
						iOS_video_rotate=0;
				}
				else
				{
					iOS_video_rotate--;
					if (iOS_video_rotate<0)
						iOS_video_rotate=2;
				}
				break;
			case 2:
				switch (iOS_video_depth)
				{
					case -1: iOS_video_depth=8; break;
					case 8: iOS_video_depth=16; break;
					case 16: iOS_video_depth=-1; break;
				}
				break;
			case 3:
				iOS_video_sync=iOS_video_sync+1;
				if (iOS_video_sync>1)
					iOS_video_sync=-1;
				break;
			case 4:
				/* "Frame-Skip" */
				if(ExKey & GP2X_R || ExKey & GP2X_RIGHT )
				{
					iOS_frameskip ++;
					if (iOS_frameskip>11)
						iOS_frameskip=-1;
				}
				else
				{
					iOS_frameskip--;
					if (iOS_frameskip<-1)
						iOS_frameskip=11;
				}
				break;
			case 5:
				if(ExKey & GP2X_R || ExKey & GP2X_RIGHT)
				{
					iOS_sound ++;
					if (iOS_sound>12)
						iOS_sound=0;
				}
				else
				{
					iOS_sound--;
					if (iOS_sound<0)
						iOS_sound=12;
				}
				break;
			case 6:
				switch (iOS_waysStick)
				{
					case 8: iOS_waysStick=4; break;
					case 4: iOS_waysStick=2; break;
					case 2: iOS_waysStick=8; break;
				}
				break;
			case 7:
				if(ExKey & GP2X_R || ExKey & GP2X_RIGHT)
				{
					iOS_buttons ++;
					if (iOS_buttons>6)
						iOS_buttons=0;
				}
				else
				{
					iOS_buttons--;
					if (iOS_buttons<0)
						iOS_buttons=6;
				}
				break;
			case 8:
				/* "CPU Clock" */
				if(ExKey & GP2X_R || ExKey & GP2X_RIGHT)
				{
					iOS_clock_cpu += 10; /* Add 10% */
					if (iOS_clock_cpu > 200) /* 200% is the max */
						iOS_clock_cpu = 200;
				}
				else
				{
					iOS_clock_cpu -= 10; /* Subtract 10% */
					if (iOS_clock_cpu < 10) /* 10% is the min */
						iOS_clock_cpu = 10;
				}
				break;
			case 9:
				/* "Audio Clock" */
				if(ExKey & GP2X_R || ExKey & GP2X_RIGHT)
				{
					iOS_clock_sound += 10; /* Add 10% */
					if (iOS_clock_sound > 200) /* 200% is the max */
						iOS_clock_sound = 200;
				}
				else{
					iOS_clock_sound -= 10; /* Subtract 10% */
					if (iOS_clock_sound < 10) /* 10% is the min */
						iOS_clock_sound = 10;
				}
				break;
			case 10:
				iOS_cheat=!iOS_cheat;
				break;
			}
		}

		if ((ExKey & GP2X_A) || (ExKey & GP2X_B) || (ExKey & GP2X_PUSH) || (ExKey & GP2X_START))
		{
			/* Write game configuration */
			sprintf(text,get_documents_path("iOS/%s_v4.cfg"),game);
			f=fopen(text,"w");
			if (f) {
				fprintf(f,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",iOS_video_aspect,iOS_video_rotate,iOS_video_sync,
				iOS_frameskip,iOS_sound,iOS_buttons,iOS_clock_cpu,iOS_clock_sound,i,iOS_cheat,iOS_video_depth,iOS_waysStick);
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

	unsigned long ExKey=0;
    int keydelay=0;

	/* No Selected game */
	strcpy(game,"builtinn");

	/* Clean screen */
	gp2x_video_flip();

	if(!safe_render_path)
	   while(ExKey=gp2x_joystick_read(0)&0x8c0ff55){usleep(1000);};

	/* Wait until user selects a game */
	while(1)
	{
        
        if(local_manufacturer != global_manufacturer || local_clones != global_clones 
			|| local_filter != global_filter || local_category != global_category
            || local_year != global_year
            || local_game_list_num != game_list_num) {
            game_list_init();
            last_game_selected=0;
            local_manufacturer = global_manufacturer;
            local_clones = global_clones;
            local_filter = global_filter;
            local_category = global_category;
            local_year = global_year;
        }
        
		game_list_view(&last_game_selected);
		gp2x_video_flip();

        if(keydelay) {
            usleep(400000);
            keydelay=0;
        } 
        
        if(safe_render_path)
        {          
			if( (gp2x_joystick_read(0)&0x8c0ff55)) {
                usleep(100000);
            }
			while(!(ExKey=gp2x_joystick_read(0)&0x8c0ff55)
                  && local_manufacturer == global_manufacturer
				  && local_clones == global_clones
  			      && local_filter == global_filter
				  && local_category == global_category
                  && local_year == global_year
                  && local_game_list_num == game_list_num)
			{
                keydelay=1;
                usleep(1000);
			}
        }
        else
        {
            usleep(100000);
        	ExKey=gp2x_joystick_read(0);
        }
  
        
		if (ExKey & GP2X_UP) last_game_selected--;
		else if (ExKey & GP2X_DOWN) last_game_selected++;
		else if ((ExKey & GP2X_L) || ExKey & GP2X_LEFT) last_game_selected-=21;
		else if ((ExKey & GP2X_R) || ExKey & GP2X_RIGHT) last_game_selected+=21;
		//if ((ExKey & GP2X_L) && (ExKey & GP2X_R)) gp2x_exit();
        
        //Set or clear favorite setting for this game
        if (ExKey & GP2X_SELECT) {
            //Check if the game is already a favorite
            game_list_select(last_game_selected, game, emu);
            
            int foundfav=0;
            int counter=0;
            while(true) {
                if (favarray[counter][0] == '\0') break;	//Null is the array terminator
                if (strcasecmp(favarray[counter], game) == 0) {
                    foundfav=1;
                    break;
                }
                counter++;
            }     
            
            if(foundfav) {
                favorites_remove(game);
            } else {
                favorites_add(game);
            }
            
        }

		if (((ExKey & GP2X_A) || (ExKey & GP2X_B) || (ExKey & GP2X_PUSH) || (ExKey & GP2X_START)) && game_num_avail!=0)
		{
			/* Select the game */
			game_list_select(last_game_selected, game, emu);

			/* Emulation Options */
			//defaults!

			iOS_video_aspect=0;
			iOS_video_rotate=0;
			iOS_video_sync=0;
			iOS_frameskip=-1;
			iOS_cheat=0;
            iOS_waysStick = 8;

			if(!safe_render_path)
			{
				iOS_sound = global_sound;
				iOS_video_depth=8;
			}
			else
			{
				iOS_sound = global_sound;
				iOS_video_depth=16;
			}
			if(isIpad)
			{
				iOS_clock_cpu= 100;
				iOS_clock_sound= 100;
				iOS_buttons=2;
				iOS_sound=global_sound;
			}
			else
			{
				iOS_clock_cpu= 80;
				iOS_clock_sound= 80;
				iOS_buttons=2;
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
	if (iOS_video_depth==8)
	{
		args[n]="-depth"; n++;
		args[n]="8"; n++;
	}
	if (iOS_video_depth==16)
	{
		args[n]="-depth"; n++;
		args[n]="16"; n++;
	}

	/* iOS_video_aspect */
	iOS_aspectRatio = iOS_cropVideo = iOS_fixedRes = 0;
    if (iOS_video_aspect==0)
	{
    	iOS_aspectRatio = 1;
    }else if(iOS_video_aspect==1){
		iOS_cropVideo = 1;
    }else if(iOS_video_aspect==2){
		iOS_cropVideo = 2;
    }else if(iOS_video_aspect==3){
		iOS_fixedRes = 1;
		//printf("fixed %d,%d,%d\n",iOS_aspectRatio,iOS_cropVideo,iOS_320x240);
    }else if(iOS_video_aspect==4){
		iOS_fixedRes = 2;
    }else if(iOS_video_aspect==5){
		iOS_fixedRes = 3;
    }else if(iOS_video_aspect==6){
		iOS_fixedRes = 4;
    }

	/* iOS_video_rotate */
	if ((iOS_video_rotate>=1) && (iOS_video_rotate<=2))
	{
		args[n]="-ror"; n++;
	}

	if ((iOS_video_rotate>=2) && (iOS_video_rotate<=2))
	{
		args[n]="-rotatecontrols"; n++;
	}
	
	/* iOS_video_sync */
    if (iOS_video_sync==1)
	{
		args[n]="-nodirty"; n++;
	}
	else if (iOS_video_sync==-1)
	{
		args[n]="-nothrottle"; n++;
	}
	
	/* iOS_frameskip */
	if (iOS_frameskip>=0)
	{
		args[n]="-frameskip"; n++;
		sprintf(str[i],"%d",iOS_frameskip);
		args[n]=str[i]; i++; n++;
	}

	/* iOS_sound */
	if (iOS_sound==0)
	{
		args[n]="-soundcard"; n++;
		args[n]="0"; n++;
	}
	if ((iOS_sound==1) || (iOS_sound==5) || (iOS_sound==9))
	{
		args[n]="-samplerate"; n++;
		args[n]="11025"; n++;
	}
	if ((iOS_sound==2) || (iOS_sound==6) || (iOS_sound==10))
	{
		args[n]="-samplerate"; n++;
		args[n]="22050"; n++;
	}
	if ((iOS_sound==3) || (iOS_sound==7) || (iOS_sound==11))
	{
		args[n]="-samplerate"; n++;
		args[n]="32000"; n++;
	}
	if ((iOS_sound==4) || (iOS_sound==8) || (iOS_sound==12))
	{
		args[n]="-samplerate"; n++;
		args[n]="44100"; n++;
	}

	if ((iOS_sound>=1) && (iOS_sound<=4))
	{
		args[n]="-fastsound"; n++;
	}
	if (iOS_sound>=9)
	{
		args[n]="-stereo"; n++;
	}

	/* iOS_clock_cpu */
	if (iOS_clock_cpu!=100)
	{
		args[n]="-uclock"; n++;
		sprintf(str[i],"%d",100-iOS_clock_cpu);
		args[n]=str[i]; i++; n++;
	}

	/* iOS_clock_sound */
	if (iOS_clock_cpu!=100)
	{
		args[n]="-uclocks"; n++;
		sprintf(str[i],"%d",100-iOS_clock_sound);
		args[n]=str[i]; i++; n++;
	}

	if (iOS_cheat)
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
	}
	printf("\n");
	
	iOS_inGame = 1;
	iOS_exitGame=0;
	iOS_hide_LR = iOS_buttons!=6;
	iOS_BplusX = iOS_buttons==3;
	iOS_landscape_buttons = iOS_buttons <= 3 ? iOS_buttons : (iOS_buttons - 1) ;
	//gp2x_set_video_mode(16,320,240);
	iphone_main(n, args);

	if(isIpad)
		iOS_buttons=2;
	else
		iOS_buttons=2;

	iOS_hide_LR = 0;
	iOS_BplusX = 0;

	iOS_exitGame=0;
	iOS_inGame = 0;
	iOS_landscape_buttons = 2;
	emulated_width = 320;
	emulated_height = 240;
	gp2x_set_video_mode(16,320,240);

}


extern "C" int mimain (int argc, char **argv)
{
	FILE *f;


	printf("init iOS frontend\n");
	/* GP2X Initialization */
	gp2x_init(1000,8,22050,16,0,60);

	//hack: por defecto lentos van a 11000
	if(!safe_render_path)
	{
		iOS_sound = global_sound;
		iOS_video_depth = 8;
	}
	else
	{
		iOS_sound = global_sound;
		iOS_video_depth = 16;
	}

	if(isIpad)
	{
		iOS_clock_cpu= 100;
		iOS_clock_sound= 100;
		iOS_buttons=2;
		iOS_sound=global_sound;
	}

	/* Show intro screen */
	gp2x_intro_screen();

    local_manufacturer = global_manufacturer;
    local_clones = global_clones;
    local_filter = global_filter;
    local_category = global_category;
    local_year = global_year;
    
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
	f=fopen(get_documents_path("iOS/mame_v2.cfg"),"r");
	if (f) {
		fscanf(f,"%d",&last_game_selected);
		fclose(f);
	}
	
	/* Select Game */
	select_game(playemu,playgame); 

	/* Write default configuration */
	f=fopen(get_documents_path("iOS/mame_v2.cfg"),"w");
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
