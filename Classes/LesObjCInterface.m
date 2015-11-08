//
//  LesObjCInterface.m
//  iMAME4all
//
//  Created by Les Bird on 12/1/13.
//
//

#import "LesObjCInterface.h"
#import "EmulatorController.h"
#import "minimal.h"

#import <GameController/GameController.h>

static GCController *gameController;
static BOOL usingTVRemote;
static int selectPressed;
extern unsigned long gp2x_pad_status;
extern int btnStates[NUM_BUTTONS];
extern int dpad_state;
extern unsigned long iCadeUsed;
float mfi_analog_x[4];
float mfi_analog_y[4];
extern int iOS_exitGame;
extern int iOS_inGame;

@implementation LesObjCInterface

+(const char *)getBundlePath
{
	const char *userPath = [[[NSBundle mainBundle] bundlePath] cStringUsingEncoding:NSASCIIStringEncoding];
	return userPath;
}

+(const char *)getDocumentsPath
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	const char *userPath = [[paths objectAtIndex:0] cStringUsingEncoding:NSASCIIStringEncoding];
	
	return userPath;
}

+(const char *)getRomPath
{
    
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	const char *userPath = [[paths objectAtIndex:0] cStringUsingEncoding:NSASCIIStringEncoding];
	
	return userPath;
}


+(void)pollMFIController
{
	if (gameController == nil)
	{
		if ([[GCController controllers] count] == 0)
		{
			return;
		}
        
        // Default to a non TV remote
		for (int i = 0; i < [[GCController controllers] count]; i++)
		{
			GCController *controller = [[GCController controllers] objectAtIndex:i];
			if (controller != nil && ![controller.vendorName isEqualToString:@"Remote"])
			{
				gameController = controller;
				iCadeUsed = YES; // set this to turn off the virtual joysticks
                usingTVRemote=false;
                gameController.controllerPausedHandler = ^(GCController *controller)
                {
                    iOS_exitGame = 1;
                    
                };
				break;
			}
		}
	}
    
    if (gameController==nil) {
        printf("No controller detected - Switching to tvRemote :-)");
        usingTVRemote=true;
        gameController = [[GCController controllers] objectAtIndex:0];
        gameController.microGamepad.allowsRotation=true;
        gameController.microGamepad.reportsAbsoluteDpadValues=TRUE;
        gameController.controllerPausedHandler = ^(GCController *controller)
        {
            iOS_exitGame = 1;
            selectPressed=0;
        };
    }
}

+(void)handleMFIController
{
	[LesObjCInterface pollMFIController];
	
	if (gameController != nil)
	{
		gp2x_pad_status &= ~GP2X_LEFT;
		gp2x_pad_status &= ~GP2X_RIGHT;
		gp2x_pad_status &= ~GP2X_UP;
		gp2x_pad_status &= ~GP2X_DOWN;
		gp2x_pad_status &= ~GP2X_X;
		btnStates[BTN_X] = BUTTON_NO_PRESS;
		gp2x_pad_status &= ~GP2X_B;
		btnStates[BTN_B] = BUTTON_NO_PRESS;
		gp2x_pad_status &= ~GP2X_A;
		btnStates[BTN_A] = BUTTON_NO_PRESS;
		gp2x_pad_status &= ~GP2X_Y;
		btnStates[BTN_Y] = BUTTON_NO_PRESS;

        if (usingTVRemote) {

            if (gameController.microGamepad.buttonX.isPressed && gameController.microGamepad.buttonA.isPressed && selectPressed<20)
            {
                selectPressed++;
                if (selectPressed<5){
                    printf("Select");
                    gp2x_pad_status |= GP2X_SELECT;
                    btnStates[BTN_SELECT] = BUTTON_PRESS;
                    return;
                } else {
                    printf("Start");
                    gp2x_pad_status |= GP2X_START;
                    btnStates[BTN_START] = BUTTON_PRESS;
                    return;
                }
                return;
            }
                
            
            if (gameController.microGamepad.buttonA.isPressed) {
                printf("X");
                gp2x_pad_status |= GP2X_X;
                btnStates[BTN_X] = BUTTON_PRESS;
            }
            
            if (gameController.microGamepad.buttonX.isPressed) {
                printf("B");
                gp2x_pad_status |= GP2X_B;
                btnStates[BTN_B] = BUTTON_PRESS;
            }
            
            const float sensitivity=.1;
            
            /*
            if (gameController.microGamepad.dpad.left.value>=sensitivity &&
                gameController.microGamepad.dpad.up.value>=sensitivity) {
                printf("Up/Left %f",gameController.microGamepad.dpad.left.value);
                gp2x_pad_status |= GP2X_LEFT;
                gp2x_pad_status |= GP2X_UP;
                return;
            }
 
            if (gameController.microGamepad.dpad.right.value>=sensitivity &&
                gameController.microGamepad.dpad.up.value>=sensitivity) {
                printf("Up/Right %f",gameController.microGamepad.dpad.left.value);
                gp2x_pad_status |= GP2X_LEFT;
                gp2x_pad_status |= GP2X_UP;
                return;
            }
            if (gameController.microGamepad.dpad.right.value>=sensitivity &&
                gameController.microGamepad.dpad.down.value>=sensitivity) {
                printf("Down/Right %f",gameController.microGamepad.dpad.left.value);
                gp2x_pad_status |= GP2X_LEFT;
                gp2x_pad_status |= GP2X_UP;
                return;
            }
            if (gameController.microGamepad.dpad.left.value>=sensitivity &&
                gameController.microGamepad.dpad.down.value>=sensitivity) {
                printf("Down/Left %f",gameController.microGamepad.dpad.left.value);
                gp2x_pad_status |= GP2X_LEFT;
                gp2x_pad_status |= GP2X_UP;
                return;
            }
             */
            
            if (gameController.microGamepad.dpad.left.value>=sensitivity) {
                printf("Left %f",gameController.microGamepad.dpad.left.value);
                gp2x_pad_status |= GP2X_LEFT;
                //return;
            }
            //
            if (gameController.microGamepad.dpad.right.value>=sensitivity) {
                printf("Right %f",gameController.microGamepad.dpad.right.value);
                gp2x_pad_status |= GP2X_RIGHT;
                //return;
            }
            if (gameController.microGamepad.dpad.up.value>=sensitivity) {
                printf("Up");
                gp2x_pad_status |= GP2X_UP;
                //return;
            }
            if (gameController.microGamepad.dpad.down.value>=sensitivity) {
                printf("Down");
                gp2x_pad_status |= GP2X_DOWN;
                //return;
            }
            
            return;
        }
        
        
		if (gameController.gamepad.dpad.up.isPressed)
		{
			gp2x_pad_status |= GP2X_UP;
		}
		
		if (gameController.gamepad.dpad.down.isPressed)
		{
			gp2x_pad_status |= GP2X_DOWN;
		}
		
		if (gameController.gamepad.dpad.left.isPressed)
		{
			gp2x_pad_status |= GP2X_LEFT;
		}
		
		if (gameController.gamepad.dpad.right.isPressed)
		{
			gp2x_pad_status |= GP2X_RIGHT;
		}
		
		if (gameController.gamepad.buttonA.isPressed)
		{
            gp2x_pad_status |= GP2X_X;
            btnStates[BTN_X] = BUTTON_PRESS;
		}
		
		if (gameController.gamepad.buttonB.isPressed)
		{
            gp2x_pad_status |= GP2X_B;
            btnStates[BTN_B] = BUTTON_PRESS;
		}
		
		if (gameController.gamepad.buttonX.isPressed)
		{
            gp2x_pad_status |= GP2X_A;
            btnStates[BTN_A] = BUTTON_PRESS;
		}
        
     	
		if (gameController.gamepad.buttonY.isPressed)
		{
            gp2x_pad_status |= GP2X_Y;
            btnStates[BTN_Y] = BUTTON_PRESS;
		}
		
		if (gameController.gamepad.leftShoulder.isPressed)
		{
            gp2x_pad_status |= GP2X_SELECT;
            btnStates[BTN_SELECT] = BUTTON_PRESS;
		}
		else
		{
            gp2x_pad_status &= ~GP2X_SELECT;
            btnStates[BTN_SELECT] = BUTTON_NO_PRESS;
		}
		
		if (gameController.gamepad.rightShoulder.isPressed)
		{
            gp2x_pad_status |= GP2X_START;
            btnStates[BTN_START] = BUTTON_PRESS;
		}
		else
		{
            gp2x_pad_status &= ~GP2X_START;
            btnStates[BTN_START] = BUTTON_NO_PRESS;
		}
		
		GCExtendedGamepad *extpad = gameController.extendedGamepad;
		if (extpad != nil)
		{
			float x1,y1,x2,y2;
			x1 = extpad.leftThumbstick.xAxis.value;
			mfi_analog_x[0] = x1;
			
			if (x1 < 0)
			{
				gp2x_pad_status |= GP2X_LEFT;
			}
			
			if (x1 > 0)
			{
				gp2x_pad_status |= GP2X_RIGHT;
			}
			
			y1 = extpad.leftThumbstick.yAxis.value;
			mfi_analog_y[0] = y1;
			
			if (y1 > 0)
			{
				gp2x_pad_status |= GP2X_UP;
			}

			if (y1 < 0)
			{
				gp2x_pad_status |= GP2X_DOWN;
			}

			x2 = extpad.rightThumbstick.xAxis.value;
			mfi_analog_x[1] = x2;
			
			if (x2 < 0)
			{
				gp2x_pad_status |= GP2X_A;
				btnStates[BTN_A] = BUTTON_PRESS;
			}
			
			if (x2 > 0)
			{
				gp2x_pad_status |= GP2X_B;
				btnStates[BTN_B] = BUTTON_PRESS;
			}

			y2 = extpad.rightThumbstick.yAxis.value;
			mfi_analog_y[1] = y2;
			
			if (y2 < 0)
			{
				gp2x_pad_status |= GP2X_X;
				btnStates[BTN_X] = BUTTON_PRESS;
			}
			
			if (y2 > 0)
			{
				gp2x_pad_status |= GP2X_Y;
				btnStates[BTN_Y] = BUTTON_PRESS;
			}
		}
	}
}

@end

const char *getBundleFolder()
{
    return [LesObjCInterface getBundlePath];
}

const char *getDocumentsFolder()
{
    return [LesObjCInterface getDocumentsPath];
}

const char *getRomFolder()
{
    //return [LesObjCInterface getRomPath];
    return [LesObjCInterface getBundlePath];
}

void pollMFIController()
{
	return [LesObjCInterface pollMFIController];
}

bool hasMFIController()
{
	if (gameController != nil)
	{
		return true;
	}
	return false;
}

void handleMFIController()
{
	[LesObjCInterface handleMFIController];
}
