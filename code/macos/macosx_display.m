/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#import "macosx_display.h"

#include "tr_local.h"
#import "macosx_local.h"

#import <Foundation/Foundation.h>
#import <IOKit/graphics/IOGraphicsTypes.h>  // for interpreting the kCGDisplayIOFlags element of the display mode


NSDictionary *Sys_GetMatchingDisplayMode(qboolean allowStretchedModes)
{
    NSArray *displayModes;
    NSDictionary *mode;
    unsigned int modeIndex, modeCount, bestModeIndex;
    int verbose;
    cvar_t *cMinFreq, *cMaxFreq;
    int minFreq, maxFreq;
    unsigned int colorDepth;
    
    verbose = r_verbose->integer;

    colorDepth = r_colorbits->integer;
    if (colorDepth < 16 || !r_fullscreen->integer)
        colorDepth = [[glw_state.desktopMode objectForKey: (id)kCGDisplayBitsPerPixel] intValue];

    cMinFreq = ri.Cvar_Get("r_minDisplayRefresh", "0", CVAR_ARCHIVE);
    cMaxFreq = ri.Cvar_Get("r_maxDisplayRefresh", "0", CVAR_ARCHIVE);

    if (cMinFreq && cMaxFreq && cMinFreq->integer && cMaxFreq->integer &&
        cMinFreq->integer > cMaxFreq->integer) {
        ri.Error(ERR_FATAL, "r_minDisplayRefresh must be less than or equal to r_maxDisplayRefresh");
    }

    minFreq = cMinFreq ? cMinFreq->integer : 0;
    maxFreq = cMaxFreq ? cMaxFreq->integer : 0;
    
    displayModes = (NSArray *)CGDisplayAvailableModes(glw_state.display);
    if (!displayModes) {
        ri.Error(ERR_FATAL, "CGDisplayAvailableModes returned NULL -- 0x%0x is an invalid display", glw_state.display);
    }
    
    modeCount = [displayModes count];
    if (verbose) {
        ri.Printf(PRINT_ALL, "%d modes avaliable\n", modeCount);
        ri.Printf(PRINT_ALL, "Current mode is %s\n", [[(id)CGDisplayCurrentMode(glw_state.display) description] cString]);
    }
    
    // Default to the current desktop mode
    bestModeIndex = 0xFFFFFFFF;
    
    for ( modeIndex = 0; modeIndex < modeCount-1; modeIndex++ ) {
        id object;
        int refresh;
        
        mode = [displayModes objectAtIndex: modeIndex];
        if (verbose) {
            ri.Printf(PRINT_ALL, " mode %d -- %s\n", modeIndex, [[mode description] cString]);
        }

        // Make sure we get the right size
        object = [mode objectForKey: (id)kCGDisplayWidth];

        if ([[mode objectForKey: (id)kCGDisplayWidth] intValue] != glConfig.vidWidth ||
            [[mode objectForKey: (id)kCGDisplayHeight] intValue] != glConfig.vidHeight) {
            if (verbose)
                ri.Printf(PRINT_ALL, " -- bad size\n");
            continue;
        }

        if (!allowStretchedModes) {
            if ([[mode objectForKey: (id)kCGDisplayIOFlags] intValue] & kDisplayModeStretchedFlag) {
                if (verbose)
                    ri.Printf(PRINT_ALL, " -- stretched modes disallowed\n");
                continue;
            }
        }

        // Make sure that our frequency restrictions are observed
        refresh = [[mode objectForKey: (id)kCGDisplayRefreshRate] intValue];
        if (minFreq &&  refresh < minFreq) {
            if (verbose)
                ri.Printf(PRINT_ALL, " -- refresh too low\n");
            continue;
        }

        if (maxFreq && refresh > maxFreq) {
            if (verbose)
                ri.Printf(PRINT_ALL, " -- refresh too high\n");
            continue;
        }

        if ([[mode objectForKey: (id)kCGDisplayBitsPerPixel] intValue] != colorDepth) {
            if (verbose)
                ri.Printf(PRINT_ALL, " -- bad depth\n");
            continue;
        }

        bestModeIndex = modeIndex;
        if (verbose)
            ri.Printf(PRINT_ALL, " -- OK\n", bestModeIndex);
    }

    if (verbose)
        ri.Printf(PRINT_ALL, " bestModeIndex = %d\n", bestModeIndex);

    if (bestModeIndex == 0xFFFFFFFF) {
        ri.Printf(PRINT_ALL, "No suitable display mode available.\n");
        return nil;
    }
    
    return [displayModes objectAtIndex: bestModeIndex];
}



