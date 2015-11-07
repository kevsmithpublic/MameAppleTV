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

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#import "CoreSurface/CoreSurface.h"
#import <QuartzCore/CALayer.h>
#import "DView.h"
#import "AnalogStick.h"

#import <pthread.h>
#import <sched.h>
#import <unistd.h>
#import <sys/time.h>

@import GameController;

#define NUM_BUTTONS 10

#ifdef IOS3
@protocol UIKeyInput <UITextInputTraits>

- (BOOL)hasText;
- (void)insertText:(NSString *)text;
- (void)deleteBackward;

@end

typedef enum {
    UIModalPresentationFullScreen = 0,
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_3_2
    UIModalPresentationPageSheet,
    UIModalPresentationFormSheet,
    UIModalPresentationCurrentContext,
#endif
} UIModalPresentationStyle;
#endif

#define STICK4WAY (iOS_waysStick == 4 && iOS_inGame)
#define STICK2WAY (iOS_waysStick == 2 && iOS_inGame)

enum { DPAD_NONE=0,DPAD_UP=1,DPAD_DOWN=2,DPAD_LEFT=3,DPAD_RIGHT=4,DPAD_UP_LEFT=5,DPAD_UP_RIGHT=6,DPAD_DOWN_LEFT=7,DPAD_DOWN_RIGHT=8};

enum { BTN_B=0,BTN_X=1,BTN_A=2,BTN_Y=3,BTN_SELECT=4,BTN_START=5,BTN_L1=6,BTN_R1=7,BTN_L2=8,BTN_R2=9};

enum { BUTTON_PRESS=0,BUTTON_NO_PRESS=1};

@class iCadeView;

const char* get_resource_path(char* file);

@interface EmulatorController : GCEventViewController
{

  UIView			* screenView;
  UIImageView	    * imageBack;
  UIImageView	    * imageOverlay;
  DView             * dview;
  //NSTimer           * touchTimer;
  @public UIView	*externalView;

  UIImageView	    * dpadView;
  UIImageView	    * buttonViews[NUM_BUTTONS];

  AnalogStickView   * analogStickView;
    
  iCadeView         *iCade;  

  NSObject     * menu;


  //joy controller
  CGRect ButtonUp;
  CGRect ButtonLeft;
  CGRect ButtonDown;
  CGRect ButtonRight;
  CGRect ButtonUpLeft;
  CGRect ButtonDownLeft;
  CGRect ButtonUpRight;
  CGRect ButtonDownRight;
  CGRect Up;
  CGRect Left;
  CGRect Down;
  CGRect Right;
  CGRect UpLeft;
  CGRect DownLeft;
  CGRect UpRight;
  CGRect DownRight;
  CGRect Select;
  CGRect Start;
  CGRect LPad;
  CGRect RPad;
  CGRect LPad2;
  CGRect RPad2;
  CGRect Menu;

  //buttons & Dpad images
  CGRect rDPad_image;
  NSString *nameImgDPad[9];

  CGRect rButton_image[NUM_BUTTONS];

  NSString *nameImgButton_Press[NUM_BUTTONS];
  NSString *nameImgButton_NotPress[NUM_BUTTONS];

  //NSTimer						*	timer;
}



- (void)getControllerCoords:(int)orientation;

- (void)getConf;
- (void)filldrectsController;

- (void)startEmulation;

- (void)removeDPadView;
- (void)buildDPadView;

- (void)changeUI;

- (void)buildPortraitImageBack;
- (void)buildPortraitImageOverlay;
- (void)buildPortrait;
- (void)buildLandscapeImageOverlay;
- (void)buildLandscapeImageBack;
- (void)buildLandscape;

- (void)runMenu;
- (void)endMenu;

- (void)handle_DPAD;
- (void)handle_MENU;

- (void)touchesController:(NSSet *)touches withEvent:(UIEvent *)event;

@property (readwrite,assign)   UIView	 *externalView;

@end
