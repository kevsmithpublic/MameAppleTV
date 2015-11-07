/*
 * This file is part of iMAME4all.
 *
 * Copyright (C) 2013 David Valdeita (Seleuco)
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

#import "iCadeView.h"

#include "minimal.h"
#import "EmulatorController.h"

extern int iOS_waysStick;
extern int iOS_inGame;
extern unsigned long gp2x_pad_status;
extern int btnStates[NUM_BUTTONS];
extern int iOS_iCadeLayout;
extern int dpad_state;
extern unsigned long iCadeUsed;

@implementation iCadeView

@synthesize active;

- (id)initWithFrame:(CGRect)frame withEmuController:(EmulatorController*)emulatorController
{
    self = [super initWithFrame:frame];
    inputView = [[UIView alloc] initWithFrame:CGRectZero];//inputView es variable de instancia que ya elimina el super
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didEnterBackground) name:UIApplicationDidEnterBackgroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didBecomeActive) name:UIApplicationDidBecomeActiveNotification object:nil];
    
    emuController = emulatorController;
    
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationDidEnterBackgroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationDidBecomeActiveNotification object:nil];
    [super dealloc];
}

- (void)didEnterBackground {
    if (self.active)
    {
        [self resignFirstResponder];
    }
}

- (void)didBecomeActive {
    if (self.active)
    {
        [self becomeFirstResponder];
    }

    if(iCadeUsed){//ensure is iCade
        iCadeUsed = 0;
        [emuController changeUI];
    }
}

- (BOOL)canBecomeFirstResponder {
    return YES;
}

- (void)setActive:(BOOL)value {
    if (active == value) return;
    
    active = value;
    if (active) {
        [self becomeFirstResponder];
    } else {
        [self resignFirstResponder];
    }
}


- (UIView*) inputView {
    return inputView;
}

#pragma mark -
#pragma mark UIKeyInput Protocol Methods

- (BOOL)hasText {
    return NO;
}

- (void)insertText:(NSString *)text {
    //NSLog(@"%s: %@ %d", __FUNCTION__, text, [text characterAtIndex:0]);
    static int up = 0;
    static int down = 0;
    static int left = 0;
    static int right = 0;
    
    unichar key = [text characterAtIndex:0];
    
    if(iCadeUsed == 0)
    {
        iCadeUsed = 1;
        [emuController changeUI];
    }
    
    switch (key)
    {            
            // joystick up
        case 'w':
            //if(!STICK2WAY && !(STICK4WAY && (left || right)))
            if(STICK4WAY)
            {
                gp2x_pad_status &= ~GP2X_LEFT;
                gp2x_pad_status &= ~GP2X_RIGHT;
            }
            if(!STICK2WAY)
                gp2x_pad_status |= GP2X_UP;
            up = 1;
            break;
        case 'e':
            if(STICK4WAY)
            {
                if(left)gp2x_pad_status |= GP2X_LEFT;
                if(right)gp2x_pad_status |= GP2X_RIGHT;
            }
            gp2x_pad_status &= ~GP2X_UP;
            up = 0;
            break;
            
            // joystick down
        case 'x':
            //if(!STICK2WAY && !(STICK4WAY && (left || right)))
            if(STICK4WAY)
            {
                gp2x_pad_status &= ~GP2X_LEFT;
                gp2x_pad_status &= ~GP2X_RIGHT;
            }
            if(!STICK2WAY)
                gp2x_pad_status |= GP2X_DOWN;
            down = 1;
            break;
        case 'z':
            if(STICK4WAY)
            {
                if(left)gp2x_pad_status |= GP2X_LEFT;
                if(right)gp2x_pad_status |= GP2X_RIGHT;
            }
            gp2x_pad_status &= ~GP2X_DOWN;
            down = 0;
            break;
            
            // joystick right
        case 'd':
            if(STICK4WAY)
            {
                gp2x_pad_status &= ~GP2X_UP;
                gp2x_pad_status &= ~GP2X_DOWN;
            }
            gp2x_pad_status |= GP2X_RIGHT;
            right = 1;
            break;
        case 'c':
            if(STICK4WAY)
            {
                if(up)gp2x_pad_status |= GP2X_UP;
                if(down)gp2x_pad_status |= GP2X_DOWN;
            }
            gp2x_pad_status &= ~GP2X_RIGHT;
            right = 0;
            break;
            
            // joystick left
        case 'a':
            if(STICK4WAY)
            {
                gp2x_pad_status &= ~GP2X_UP;
                gp2x_pad_status &= ~GP2X_DOWN;
            }
            gp2x_pad_status |= GP2X_LEFT;
            left = 1;
            break;
        case 'q':
            if(STICK4WAY)
            {
                if(up)gp2x_pad_status |= GP2X_UP;
                if(down)gp2x_pad_status |= GP2X_DOWN;
            }
            gp2x_pad_status &= ~GP2X_LEFT;
            left = 0;
            break;
            
            // Y / UP
        case 'i':
            gp2x_pad_status |= GP2X_Y;
            btnStates[BTN_Y] = BUTTON_PRESS;
            break;
        case 'm':
            gp2x_pad_status &= ~GP2X_Y;
            btnStates[BTN_Y] = BUTTON_NO_PRESS;
            break;
            
            // X / DOWN
        case 'l':
            gp2x_pad_status |= GP2X_X;
            btnStates[BTN_X] = BUTTON_PRESS;
            break;
        case 'v':
            gp2x_pad_status &= ~GP2X_X;
            btnStates[BTN_X] = BUTTON_NO_PRESS;
            break;
            
            // A / LEFT
        case 'k':
            gp2x_pad_status |= GP2X_A;
            btnStates[BTN_A] = BUTTON_PRESS;
            break;
        case 'p':
            gp2x_pad_status &= ~GP2X_A;
            btnStates[BTN_A] = BUTTON_NO_PRESS;
            break;
            
            // B / RIGHT
        case 'o':
            gp2x_pad_status |= GP2X_B;
            btnStates[BTN_B] = BUTTON_PRESS;
            break;
        case 'g':
            gp2x_pad_status &= ~GP2X_B;
            btnStates[BTN_B] = BUTTON_NO_PRESS;
            break;
            
            // SELECT / COIN
        case 'y': //button down
            gp2x_pad_status |= GP2X_SELECT;
            btnStates[BTN_SELECT] = BUTTON_PRESS;
            break;
        case 't': //button up
            gp2x_pad_status &= ~GP2X_SELECT;
            btnStates[BTN_SELECT] = BUTTON_NO_PRESS;
            break;
            
            // START
        case 'u':   //button down
            if(iOS_iCadeLayout) {
                gp2x_pad_status |= GP2X_L;
                btnStates[BTN_L1] = BUTTON_PRESS;
            }
            else {
                gp2x_pad_status |= GP2X_START;
                btnStates[BTN_START] = BUTTON_PRESS;
            }
            break;
        case 'f':   //button up
            if(iOS_iCadeLayout) {
                gp2x_pad_status &= ~GP2X_L;
                btnStates[BTN_L1] = BUTTON_NO_PRESS;
            }
            else {
                gp2x_pad_status &= ~GP2X_START;
                btnStates[BTN_START] = BUTTON_NO_PRESS;
            }
            break;
            
            //
        case 'h':   //button down
            if(iOS_iCadeLayout) {
                gp2x_pad_status |= GP2X_START;
                btnStates[BTN_START] = BUTTON_PRESS;
            }
            else {
                gp2x_pad_status |= GP2X_L;
                btnStates[BTN_L1] = BUTTON_PRESS;
            }
            break;
        case 'r':   //button up
            if(iOS_iCadeLayout) {
                gp2x_pad_status &= ~GP2X_START;
                btnStates[BTN_START] = BUTTON_NO_PRESS;
            }
            else {
                gp2x_pad_status &= ~GP2X_L;
                btnStates[BTN_L1] = BUTTON_NO_PRESS;
            }
            break;
            
            //
        case 'j':
            gp2x_pad_status |= GP2X_R;
            btnStates[BTN_R1] = BUTTON_PRESS;
            break;
        case 'n':
            gp2x_pad_status &= ~GP2X_R;
            btnStates[BTN_R1] = BUTTON_NO_PRESS;
            break;
    }
    
    // calculate dpad_state
    switch (gp2x_pad_status & (GP2X_UP|GP2X_DOWN|GP2X_LEFT|GP2X_RIGHT))
    {
        case    GP2X_UP:    dpad_state = DPAD_UP; break;
        case    GP2X_DOWN:  dpad_state = DPAD_DOWN; break;
        case    GP2X_LEFT:  dpad_state = DPAD_LEFT; break;
        case    GP2X_RIGHT: dpad_state = DPAD_RIGHT; break;
            
        case    GP2X_UP | GP2X_LEFT:  dpad_state = DPAD_UP_LEFT; break;
        case    GP2X_UP | GP2X_RIGHT: dpad_state = DPAD_UP_RIGHT; break;
        case    GP2X_DOWN | GP2X_LEFT:  dpad_state = DPAD_DOWN_LEFT; break;
        case    GP2X_DOWN | GP2X_RIGHT: dpad_state = DPAD_DOWN_RIGHT; break;
            
        default: dpad_state = DPAD_NONE;
    }
    
    static int cycleResponder = 0;
    if (++cycleResponder > 20) {
        // necessary to clear a buffer that accumulates internally
        cycleResponder = 0;
        [self resignFirstResponder];
        [self becomeFirstResponder];        
    }
    
    [emuController handle_DPAD];
}

- (void)deleteBackward {
    // This space intentionally left blank to complete protocol
}

-(void)autoDimiss:(id)sender {
    
    NSObject *alert = (NSObject *)sender;
    [alert dismissWithClickedButtonIndex:0 animated:YES];
    [alert release];    
}

@end
