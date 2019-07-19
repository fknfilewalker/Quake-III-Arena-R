#include "tr_local.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

// Needs to be visible to Q3Controller.m.
void Sys_UpdateWindowMouseInputRect(NSWindow *window)
{
    NSRect           windowRect, screenRect;
    NSScreen        *screen;
    
    // It appears we need to flip the coordinate system here.  This means we need
    // to know the size of the screen.
    screen = [window screen];
    screenRect = [screen frame];
    windowRect = [window frame];
    windowRect.origin.y = screenRect.size.height - (windowRect.origin.y + windowRect.size.height);
    
    Sys_SetMouseInputRect(CGRectMake(windowRect.origin.x, windowRect.origin.y,
                                     windowRect.size.width, windowRect.size.height));
}    
