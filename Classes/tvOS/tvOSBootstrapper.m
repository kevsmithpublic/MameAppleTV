/*
 * This file is part of iMAME4all.
 *
 * Copyright (C) 2011-2013 David Valdeita (Seleuco)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 *
 * Linking iMAME4all statically or dynamically with other modules is
 * making a combined work based on iMAME4all. Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * In addition, as a special exception, the copyright holders of iMAME4all
 * give you permission to combine iMAME4all with free software programs
 * or libraries that are released under the GNU LGPL and with code included
 * in the standard release of MAME under the MAME License (or modified
 * versions of such code, with unchanged license). You may copy and
 * distribute such a system following the terms of the GNU GPL for iMAME4all
 * and the licenses of the other code concerned, provided that you include
 * the source code of that other code when and as the GNU GPL requires
 * distribution of source code.
 *
 * Note that people who make modified versions of iMAME4all are not
 * obligated to grant this special exception for their modified versions; it
 * is their choice whether to do so. The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception.
 *
 * iMAME4all is dual-licensed: Alternatively, you can license iMAME4all
 * under a MAME license, as set out in http://mamedev.org/
 */

#import "Bootstrapper.h"
#import "Helper.h"
#import <UIKit/UIKit.h>
#include <stdio.h>
#include <sys/stat.h>
#include "shared.h"


#define IS_WIDESCREEN ( fabs( ( double )[ [ UIScreen mainScreen ] bounds ].size.height - ( double )568 ) < DBL_EPSILON )
#define IS_IPAD   ( [ [ [ UIDevice currentDevice ] model ] isEqualToString: @"iPad" ] )
#define IS_IPHONE ( [ [ [ UIDevice currentDevice ] model ] isEqualToString: @"iPhone" ] )
#define IS_IPOD   ( [ [ [ UIDevice currentDevice ] model ] isEqualToString: @"iPod touch" ] )
#define IS_IPHONE_5 ( IS_IPHONE && IS_WIDESCREEN )

int isIpad = 0;
int isIphone4 = 0;
int isIphone5 = 0;
extern __emulation_run; 
CGRect rExternal;
int nativeTVOUT = 0;
int overscanTVOUT = 1;

@implementation Bootstrapper

-(void)applicationDidFinishLaunching:(NSNotification *)aNotification {

	struct CGRect rect = [[UIScreen mainScreen] bounds];
	rect.origin.x = rect.origin.y = 0.0f;
	
	//printf("Machine: '%s'\n",[[Helper machine] UTF8String]) ;
	
	//mkdir("/var/mobile/Media/ROMs/iXpectrum/downloads", 0755);
	
	mkdir(get_documents_path("iOS"), 0755);
	mkdir(get_documents_path("cfg"), 0755);
	mkdir(get_documents_path("hi"), 0755);
    mkdir(get_documents_path("snap"), 0755);
    mkdir(get_documents_path("memcard"), 0755);
    mkdir(get_documents_path("sta"), 0755);

    [[UIApplication sharedApplication] setStatusBarHidden:YES animated:NO];
    
	[[UIApplication sharedApplication] setIdleTimerDisabled:YES];//TODO ???
	
	isIpad = [[Helper machine] rangeOfString:@"iPad"].location != NSNotFound;
	isIphone4 = [[Helper machine] rangeOfString:@"iPhone3"].location != NSNotFound;
    isIphone5 = IS_WIDESCREEN;
    
    //TEST
    //isIpad = 0; isIphone5 = 0;


	hrViewController = [[EmulatorController alloc] init];
	
	deviceWindow = [[UIWindow alloc] initWithFrame:rect];
	deviceWindow.backgroundColor = [UIColor redColor];
	
    //[deviceWindow addSubview: hrViewController.view ];
	
	[deviceWindow makeKeyAndVisible];
    [deviceWindow setRootViewController:hrViewController];
	 
	externalWindow = [[UIWindow alloc] initWithFrame:CGRectZero];
	externalWindow.hidden = YES;
	 	
	if(nativeTVOUT)
	{ 	
		[[NSNotificationCenter defaultCenter] addObserver:self 
													 selector:@selector(prepareScreen) 
														 name:/*@"UIScreenDidConnectNotification"*/
                                                              UIScreenDidConnectNotification
													   object:nil];
	        
			
	    [[NSNotificationCenter defaultCenter] addObserver:self 
													 selector:@selector(prepareScreen) 
														 name:/*@"UIScreenDidDisconnectNotification"*/
                                                              UIScreenDidDisconnectNotification
													   object:nil];
	}	
    
    [self prepareScreen];
}

- (void)applicationWillResignActive:(UIApplication *)application {

   [Helper endwiimote]; 
   [hrViewController runMenu];
   usleep(1000000);   
}
/*
- (void)applicationDidBecomeActive:(UIApplication  *)application {
  
}

- (void)applicationWillTerminate:(UIApplication *)application {
 
}
*/

- (void)prepareScreen
{
	 @try
    {												   										       
	    if ([[UIScreen screens] count] > 1 && nativeTVOUT) {
	    											 	        	   					
			// Internal display is 0, external is 1.
			externalScreen = [[[UIScreen screens] objectAtIndex:1] retain];			
			screenModes =  [[externalScreen availableModes] retain];
					
			// Allow user to choose from available screen-modes (pixel-sizes).
			UIAlertView *alert = [[[UIAlertView alloc] initWithTitle:@"External Display Detected!" 
															 message:@"Choose a size for the external display." 
															delegate:self 
												   cancelButtonTitle:nil 
												   otherButtonTitles:nil] autorelease];
			for (UIScreenMode *mode in screenModes) {
				CGSize modeScreenSize = mode.size;
				[alert addButtonWithTitle:[NSString stringWithFormat:@"%.0f x %.0f pixels", modeScreenSize.width, modeScreenSize.height]];
			}
			[alert show];
			
		} else {
		     if(!__emulation_run)
		     {
		        [hrViewController startEmulation];
		     }   
		     else
		     {
		        [hrViewController setExternalView:nil];
		        externalWindow.hidden = YES;
		        [hrViewController changeUI];
		     }   
		    	
		}
	}
	 @catch(NSException* ex)
    {
        NSLog(@"Not supported tv out API!");
        if(!__emulation_run)
          [hrViewController startEmulation];
    }	
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	UIScreenMode *desiredMode = [screenModes objectAtIndex:buttonIndex];
	
	[externalScreen setCurrentMode:[screenModes objectAtIndex:buttonIndex]];
	[externalWindow setScreen:externalScreen];
	
	CGRect rect = CGRectZero;
/*	
	rect.size = desiredMode.size;
	rect.size.width =desiredMode.size.width;
	rect.size.height =desiredMode.size.height;
*/	 
	
	rect = externalScreen.bounds;
	externalWindow.frame = rect;
	externalWindow.clipsToBounds = YES;
	
	int  external_width = externalWindow.frame.size.width;
	int  external_height = externalWindow.frame.size.height;
	
	//float overscan = (desiredMode.size.width== 720) ? 0.92f : 1.0f;
	float overscan = 1 - (overscanTVOUT *  0.025f);
	
	
	//int width = (int)(external_width /* * overscan */);//overscan
    //int height = (int)((width * 3) / 4);
    
    //if(height > external_height)
    //{
    //   int  height = (int)(external_height /** overscan*/);//overscan
    //   int  width = (int)((height * 4) / 3);       
    //}
    
    //?????
    int width=external_width;
    int height=external_height; 
    /*
    	                                       UIAlertView* alert = 
                                               [[UIAlertView alloc] initWithTitle:@"Ventana"
       message:[NSString stringWithFormat:@"%@ %@ %@ %@",
       [NSNumber numberWithInt:width],
       [NSNumber numberWithInt:height],
       [NSNumber numberWithInt:overscan],
       [NSNumber numberWithInt:overscanTVOUT]
       ] 
                                                                     delegate:nil cancelButtonTitle:@"Dismiss" otherButtonTitles: nil];
                                               [alert show];                                           
                                                                                      
                                               [alert release];
    */
    width = width * overscan;    
    height = height * overscan;
    int x = (external_width - width)/2;
    int y = (external_height - height)/2;
                                       
      
    rExternal = CGRectMake( x, y, width, height);
    
    for (UIView *view in [externalWindow subviews]) {
       [view removeFromSuperview];
    }
    
    UIView *view= [[UIView alloc] initWithFrame:rect];
    view.backgroundColor = [UIColor blackColor];
    [externalWindow addSubview:view];
    [view release];
		
	[hrViewController setExternalView:view];
	externalWindow.hidden = NO;
	//[externalWindow makeKeyAndVisible];
	if(__emulation_run)
	    [hrViewController changeUI];
	else
	    [hrViewController startEmulation];
	    
    [screenModes release];
	[externalScreen release];
}

-(void)dealloc {
    [hrViewController release];
	[deviceWindow dealloc];	
	[externalWindow dealloc];
	[super dealloc];
}

@end