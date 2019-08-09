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
#import "macosx_glimp.h"

#include "tr_local.h"
#import "macosx_local.h"
#import "macosx_display.h"
#import "macosx_timers.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#import <mach-o/dyld.h>
#import <mach/mach.h>
#import <mach/mach_error.h>

cvar_t	*r_allowSoftwareGL;		// don't abort out if the pixelformat claims software
cvar_t  *r_enablerender;                // Enable actual rendering
cvar_t  *r_appleTransformHint;          // Enable Apple transform hint

static void GLW_InitExtensions( void );
static qboolean CreateGameWindow( qboolean isSecondTry );
static GLint Sys_QueryVideoMemory();
static CGDisplayErr Sys_CaptureActiveDisplays(void);


glwstate_t glw_state;
qboolean Sys_IsHidden = qfalse;

#ifdef OMNI_TIMER
OTStampList glThreadStampList;
#endif

@interface NSOpenGLContext (CGLContextAccess)
- (CGLContextObj) cglContext;
@end
/*
@implementation NSOpenGLContext (CGLContextAccess)
- (CGLContextObj) cglContext;
{
    return _contextAuxiliary;
}
@end

CGLContextObj QZ_GetCGLContextObj(NSOpenGLContext *nsctx)
{
    return (CGLContextObj) [nsctx CGLContextObj];
}*/
/*
============
CheckErrors
============
*/
void CheckErrors( void )
{		
    GLenum   err;

    err = qglGetError();
    if ( err != GL_NO_ERROR ) {
        ri.Error( ERR_FATAL, "glGetError: %s\n", qglGetString( err ) );
    }
}

#if !defined(NDEBUG) && defined(QGL_CHECK_GL_ERRORS)

unsigned int QGLBeginStarted = 0;

void QGLErrorBreak(void)
{
}

void QGLCheckError(const char *message)
{
    GLenum        error;
    static unsigned int errorCount = 0;
    
    error = _glGetError();
    if (error != GL_NO_ERROR) {
        if (errorCount == 100) {
            Com_Printf("100 GL errors printed ... disabling further error reporting.\n");
        } else if (errorCount < 100) {
            if (errorCount == 0) {
                fprintf(stderr, "BREAK ON QGLErrorBreak to stop at the GL errors\n");
            }
            fprintf(stderr, "OpenGL Error(%s): 0x%04x -- %s\n", message, (int)error,  gluErrorString(error));
            QGLErrorBreak();
        }
        errorCount++;
    }
}
#endif

/*
** GLimp_SetMode
*/

qboolean GLimp_SetMode( qboolean isSecondTry )
{
    if ( !CreateGameWindow(isSecondTry) ) {
        ri.Printf( PRINT_ALL, "GLimp_Init: window could not be created!\n" );
        return qfalse;
    }

    // draw something to show that GL is alive	
    if (r_enablerender->integer) {
        qglClearColor( 0.5, 0.5, 0.7, 0 );
        qglClear( GL_COLOR_BUFFER_BIT );
        GLimp_EndFrame();
        
        qglClearColor( 0.5, 0.5, 0.7, 0 );
        qglClear( GL_COLOR_BUFFER_BIT );
        GLimp_EndFrame();
    }
   
    CheckErrors();

    return qtrue;
}

/*
 =================
 GetPixelAttributes
 =================
 */

#define ADD_ATTR(x) \
do { \
    if (attributeIndex >= attributeSize) { \
        attributeSize *= 2; \
        pixelAttributes = NSZoneRealloc(NULL, pixelAttributes, sizeof(*pixelAttributes) * attributeSize); \
    } \
    pixelAttributes[attributeIndex] = x; \
    attributeIndex++; \
    if (verbose) { \
        ri.Printf(PRINT_ALL, "Adding pixel attribute: %d (%s)\n", x, #x); \
    } \
} while(0)

static NSOpenGLPixelFormatAttribute *GetPixelAttributes()
{
    NSOpenGLPixelFormatAttribute *pixelAttributes;
    unsigned int attributeIndex = 0;
    unsigned int attributeSize = 128;
    int verbose = 0;
    unsigned int colorDepth;
    
    verbose = r_verbose->integer;
    
    pixelAttributes = NSZoneMalloc(NULL, sizeof(*pixelAttributes) * attributeSize);

    if (r_fullscreen->integer) {
        ADD_ATTR(NSOpenGLPFAFullScreen);

        // Since we are fullscreen, specify the screen that we need.
        ADD_ATTR(NSOpenGLPFAScreenMask);
        ADD_ATTR(CGDisplayIDToOpenGLDisplayMask(Sys_DisplayToUse()));
    }
    
    // Require hardware acceleration unless otherwise directed
    if (!r_allowSoftwareGL->integer) {
        ADD_ATTR(NSOpenGLPFAAccelerated);
    }

    // Require double-buffer
    ADD_ATTR(NSOpenGLPFADoubleBuffer);

    // Specify the number of color bits.  If we don't have a valid specified value or we are not full screen, use the current display mode's value.
    ADD_ATTR(NSOpenGLPFAColorSize);
    colorDepth = r_colorbits->integer;
    if (colorDepth < 16)
        colorDepth = 16;
    else if (colorDepth > 16)
        colorDepth = 32;
    if (!r_fullscreen->integer)
        colorDepth = [[glw_state.desktopMode objectForKey: (id)kCGDisplayBitsPerPixel] intValue];
    ADD_ATTR(colorDepth);

    // Specify the number of depth bits
    ADD_ATTR(NSOpenGLPFADepthSize);
    ADD_ATTR(r_depthbits->integer ? r_depthbits->integer : 16);

    // Specify the number of stencil bits
    if (r_stencilbits->integer) {
        ADD_ATTR(NSOpenGLPFAStencilSize);
        ADD_ATTR(r_stencilbits->integer);
    } else {
        ADD_ATTR(NSOpenGLPFAStencilSize);
        ADD_ATTR(8);
    }

    // Terminate the list
    ADD_ATTR(0);
    
    return pixelAttributes;
}
								

// This is needed since CGReleaseAllDisplays() restores the gamma on the displays and we want to fade it up rather than just flickering all the displays
static void ReleaseAllDisplays()
{
    CGDisplayCount displayIndex;

    Com_Printf("Releasing displays\n");
    for (displayIndex = 0; displayIndex < glw_state.displayCount; displayIndex++) {
        CGDisplayRelease(glw_state.originalDisplayGammaTables[displayIndex].display);
    }
}

/*
=================
CreateGameWindow
=================
*/
static qboolean CreateGameWindow( qboolean isSecondTry )
{
    const char *windowed[] = { "Windowed", "Fullscreen" };
    int			current_mode;
    NSOpenGLPixelFormatAttribute *pixelAttributes;
    NSOpenGLPixelFormat *pixelFormat;
    CGDisplayErr err;
            

    // get mode info
    current_mode = r_mode->integer;
    glConfig.isFullscreen = (r_fullscreen->integer != 0);

    glw_state.desktopMode = (NSDictionary *)CGDisplayCurrentMode(glw_state.display);
    if (!glw_state.desktopMode) {
        ri.Error(ERR_FATAL, "Could not get current graphics mode for display 0x%08x\n", glw_state.display);
    }

#if 0
    ri.Printf( PRINT_ALL, "... desktop mode %d = %dx%d %s\n", glw_state.desktopMode,
               glw_state.desktopDesc.width, glw_state.desktopDesc.height,
               depthStrings[glw_state.desktopDesc.depth]);
#endif

    ri.Printf( PRINT_ALL, "...setting mode %d:\n", current_mode );
    if ( !R_GetModeInfo( &glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect, current_mode ) )  {
        ri.Printf( PRINT_ALL, " invalid mode\n" );
        return qfalse;
    }
    ri.Printf( PRINT_ALL, " %d %d %s\n", glConfig.vidWidth, glConfig.vidHeight, windowed[glConfig.isFullscreen] );

    glw_state.gameMode = glw_state.desktopMode;
    
    
    
    // Get the GL pixel format
    pixelAttributes = GetPixelAttributes();
    pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes: pixelAttributes] autorelease];
    NSZoneFree(NULL, pixelAttributes);
    
    if (!pixelFormat) {
        CGDisplayRestoreColorSyncSettings();
        CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.desktopMode);
        ReleaseAllDisplays();
        ri.Printf( PRINT_ALL, " No pixel format found\n");
        return qfalse;
    }

    // Create a context with the desired pixel attributes
    OSX_SetGLContext([[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext: nil]);
    if (!OSX_GetNSGLContext()) {
        CGDisplayRestoreColorSyncSettings();
        CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.desktopMode);
        ReleaseAllDisplays();
        ri.Printf(PRINT_ALL, "... +[NSOpenGLContext createWithFormat:share:] failed.\n" );
        return qfalse;
    }

    //if (!glConfig.isFullscreen) {
    cvar_t		*vid_xpos;
    cvar_t		*vid_ypos;
    CGRect           windowRect;
    
    vid_xpos = ri.Cvar_Get( "vid_xpos", "100", CVAR_ARCHIVE );
    vid_ypos = ri.Cvar_Get( "vid_ypos", "100", CVAR_ARCHIVE );

    // Create a window of the desired size
    windowRect.origin.x = vid_xpos->integer;
    windowRect.origin.y = vid_ypos->integer;
    windowRect.size.width = glConfig.vidWidth;
    windowRect.size.height = glConfig.vidHeight;

    if(_window == nil) _window = [[NSWindow alloc] initWithContentRect:windowRect
                                                   styleMask: (glConfig.isFullscreen ? NSTitledWindowMask : NSTitledWindowMask)
                                                     backing:NSBackingStoreRetained
                                                       defer:NO];

    glw_state.window = _window;
    NSUInteger masks = [glw_state.window styleMask];
    qboolean isFullscreen = qfalse;
    if ( masks & NSFullScreenWindowMask) {
        isFullscreen = qtrue;
    }
    
    if (glConfig.isFullscreen) {
        glw_state.gameMode = CGDisplayBestModeForParameters (kCGDirectMainDisplay, 32, glConfig.vidWidth, glConfig.vidHeight, NULL);
        err = CGDisplaySwitchToMode(glw_state.display, glw_state.gameMode);//(CFDictionaryRef)glw_state.gameMode);

        [glw_state.window toggleFullScreen:nil];
        [glw_state.window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
        [glw_state.window setFrame:windowRect display:YES];
    } else {
        
        err = CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.desktopMode);
        
        NSSize size;
        size.height = glConfig.vidHeight;
        size.width = glConfig.vidWidth;
        
        if(isFullscreen) [glw_state.window toggleFullScreen:nil];
        [glw_state.window setContentSize:size];
        [glw_state.window setCollectionBehavior:NSWindowCollectionBehaviorDefault];
        //[glw_state.window setFrame:windowRect display:YES];
    }
    
    glw_state.window.styleMask &= ~NSWindowStyleMaskResizable;
    
    if ( err != CGDisplayNoErr ) {
        ri.Printf( PRINT_ALL, " Unable to set display mode, err = %d\n", err );
        return qfalse;
    }
    
    [glw_state.window setTitle: @"Quake3"];

    [glw_state.window orderFront: nil];

    // Always get mouse moved events (if mouse support is turned off (rare)
    // the event system will filter them out.
    [glw_state.window setAcceptsMouseMovedEvents: YES];
    
    // Hider cursor
    [NSCursor hide];
    //[NSMenu setMenuBarVisible:NO];
    
    // Direct the context to draw in this window
    [OSX_GetNSGLContext() setView: [glw_state.window contentView]];

    // Sync input rect with where the window actually is...
    Sys_UpdateWindowMouseInputRect(glw_state.window);


#ifndef USE_CGLMACROS
    // Make this the current context
    OSX_GLContextSetCurrent();
#endif

    // Store off the pixel format attributes that we actually got
    [pixelFormat getValues: (GLint *) &glConfig.colorBits forAttribute: NSOpenGLPFAColorSize forVirtualScreen: 0];
    [pixelFormat getValues: (GLint *) &glConfig.depthBits forAttribute: NSOpenGLPFADepthSize forVirtualScreen: 0];
    [pixelFormat getValues: (GLint *) &glConfig.stencilBits forAttribute: NSOpenGLPFAStencilSize forVirtualScreen: 0];

    glConfig.displayFrequency = [[glw_state.gameMode objectForKey: (id)kCGDisplayRefreshRate] intValue];
    
    
    ri.Printf(PRINT_ALL, "ok\n" );

    if (glConfig.isFullscreen) {
        [glw_state.window toggleFullScreen:nil];
        
    }
    return qtrue;
}

// This can be used to temporarily disassociate the GL context from the screen so that CoreGraphics can be used to draw to the screen.
void Sys_PauseGL ()
{
    if (!glw_state.glPauseCount) {
        qglFinish (); // must do this to ensure the queue is complete
        
        // Have to call both to actually deallocate kernel resources and free the NSSurface
        CGLClearDrawable(OSX_GetCGLContext());
        [OSX_GetNSGLContext() clearDrawable];
    }
    glw_state.glPauseCount++;
}

// This can be used to reverse the pausing caused by Sys_PauseGL()
void Sys_ResumeGL ()
{
    if (glw_state.glPauseCount) {
        glw_state.glPauseCount--;
        if (!glw_state.glPauseCount) {
            [OSX_GetNSGLContext() setView: [glw_state.window contentView]];
        }
    }
}



/*
===================
GLimp_Init

Don't return unless OpenGL has been properly initialized
===================
*/

static void GLImp_Toggle_Renderer_f(void)
{
    ri.Cvar_Set("r_enablerender", r_enablerender->integer ? "0" : "1");
}

#ifdef OMNI_TIMER
static void GLImp_Dump_Stamp_List_f(void)
{
    OTStampListDumpToFile(glThreadStampList, "/tmp/gl_stamps");
}
#endif

void GLimp_Init( void )
{
    static BOOL addedCommands = NO;
    cvar_t *lastValidRenderer = ri.Cvar_Get( "r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE );
    char *buf;

    if (!addedCommands) {
        addedCommands = YES;

#ifdef OMNI_TIMER
        glThreadStampList = OTStampListCreate(64);
        Cmd_AddCommand ("dump_stamp_list", GLImp_Dump_Stamp_List_f);
#endif
        Cmd_AddCommand ("toggle_renderer", GLImp_Toggle_Renderer_f);
    }

    ri.Printf( PRINT_ALL, "Initializing OpenGL subsystem\n" );
    ri.Printf( PRINT_ALL, "  Last renderer was '%s'\n", lastValidRenderer->string);
    ri.Printf( PRINT_ALL, "  r_fullscreen = %d\n", r_fullscreen->integer);

    memset( &glConfig, 0, sizeof( glConfig ) );

    // We only allow changing the gamma if we are full screen
    glConfig.deviceSupportsGamma = false;//(r_fullscreen->integer != 0);
    
    r_allowSoftwareGL = ri.Cvar_Get( "r_allowSoftwareGL", "0", CVAR_LATCH );
    r_enablerender = ri.Cvar_Get("r_enablerender", "1", 0 );

    if (Sys_QueryVideoMemory() == 0 && !r_allowSoftwareGL->integer) {
        ri.Error( ERR_FATAL, "Could not initialize OpenGL.  There does not appear to be an OpenGL-supported video card in your system.\n" );
    }
    
    if ( ! GLimp_SetMode(qfalse) ) {
        // fall back to the known-good mode
        ri.Cvar_Set( "r_fullscreen", "1" );
        ri.Cvar_Set( "r_mode", "3" );
        ri.Cvar_Set( "r_stereo", "0" );
        ri.Cvar_Set( "r_depthBits", "16" );
        ri.Cvar_Set( "r_colorBits", "16" );
        ri.Cvar_Set( "r_stencilBits", "0" );
        if ( GLimp_SetMode(qtrue) ) {
            ri.Printf( PRINT_ALL, "------------------\n" );
            return;
        }

        ri.Error( ERR_FATAL, "Could not initialize OpenGL\n" );
        return;
    }

    ri.Printf( PRINT_ALL, "------------------\n" );

    // get our config strings
    Q_strncpyz( glConfig.vendor_string, (const char *)qglGetString (GL_VENDOR), sizeof( glConfig.vendor_string ) );
    Q_strncpyz( glConfig.renderer_string, (const char *)qglGetString (GL_RENDERER), sizeof( glConfig.renderer_string ) );
    Q_strncpyz( glConfig.version_string, (const char *)qglGetString (GL_VERSION), sizeof( glConfig.version_string ) );
    Q_strncpyz( glConfig.extensions_string, (const char *)qglGetString (GL_EXTENSIONS), sizeof( glConfig.extensions_string ) );

    //
    // chipset specific configuration
    //
    buf = malloc(strlen(glConfig.renderer_string) + 1);
    strcpy( buf, glConfig.renderer_string );
    Q_strlwr( buf );

    ri.Cvar_Set( "r_lastValidRenderer", glConfig.renderer_string );
    free(buf);

    GLW_InitExtensions();
    
#ifndef USE_CGLMACROS
    if (!r_enablerender->integer)
        OSX_GLContextClearCurrent();
#endif
}


/*
** GLimp_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void GLimp_EndFrame (void)
{
    GLSTAMP("GLimp_EndFrame start", 0);
    
    //
    // swapinterval stuff
    //
    if ( r_swapInterval->modified ) {
        r_swapInterval->modified = qfalse;

        if ( !glConfig.stereoEnabled ) {	// why?
            [[NSOpenGLContext currentContext] setValues: (GLint *)&r_swapInterval->integer
            forParameter: NSOpenGLCPSwapInterval];
        }
    }

#if !defined(NDEBUG) && defined(QGL_CHECK_GL_ERRORS)
    QGLCheckError("GLimp_EndFrame");
#endif

    if (!glw_state.glPauseCount && !Sys_IsHidden) {
        glw_state.bufferSwapCount++;
        [OSX_GetNSGLContext() flushBuffer];
    }
    
    // Enable turning off GL at any point for performance testing
    if (OSX_GLContextIsCurrent() != r_enablerender->integer) {
        if (r_enablerender->integer) {
            Com_Printf("--- Enabling Renderer ---\n");
            OSX_GLContextSetCurrent();
        } else {
            Com_Printf("--- Disabling Renderer ---\n");
            OSX_GLContextClearCurrent();
        }
    }

    GLSTAMP("GLimp_EndFrame end", 0);
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/

static void _GLimp_RestoreOriginalVideoSettings()
{
    CGDisplayErr err;
    err = CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.desktopMode);
}

void GLimp_Shutdown( void )
{
    CGDisplayCount displayIndex;

    Com_Printf("----- Shutting down GL -----\n");


    
    if (OSX_GetNSGLContext()) {
#ifndef USE_CGLMACROS
        OSX_GLContextClearCurrent();
#endif
        // Have to call both to actually deallocate kernel resources and free the NSSurface
        CGLClearDrawable(OSX_GetCGLContext());
        [OSX_GetNSGLContext() clearDrawable];
        
        [OSX_GetNSGLContext() release];
        OSX_SetGLContext((id)nil);
    }

    _GLimp_RestoreOriginalVideoSettings();
    


    // Restore the original gamma if needed.
    if (glConfig.deviceSupportsGamma) {
        Com_Printf("Restoring ColorSync settings\n");
        CGDisplayRestoreColorSyncSettings();
    }

    if (glw_state.window) {
        //[glw_state.window release];
        //[glw_state.window close];
        //glw_state.window = nil;
    }

    if (glw_state.log_fp) {
        fclose(glw_state.log_fp);
        glw_state.log_fp = 0;
    }

    for (displayIndex = 0; displayIndex < glw_state.displayCount; displayIndex++) {
        free(glw_state.originalDisplayGammaTables[displayIndex].red);
        free(glw_state.originalDisplayGammaTables[displayIndex].blue);
        free(glw_state.originalDisplayGammaTables[displayIndex].green);
    }
    free(glw_state.originalDisplayGammaTables);
    if (glw_state.tempTable.red) {
        free(glw_state.tempTable.red);
        free(glw_state.tempTable.blue);
        free(glw_state.tempTable.green);
    }
    if (glw_state.inGameTable.red) {
        free(glw_state.inGameTable.red);
        free(glw_state.inGameTable.blue);
        free(glw_state.inGameTable.green);
    }
    
    memset(&glConfig, 0, sizeof(glConfig));
    memset(&glState, 0, sizeof(glState));
    memset(&glw_state, 0, sizeof(glw_state));

    Com_Printf("----- Done shutting down GL -----\n");
}

/*
===============
GLimp_LogComment

===============
*/
void	GLimp_LogComment( char *comment ) {
        }

/*
 ===============
 GLimp_SetGamma
 
 ===============
 */
void GLimp_SetGamma(unsigned char red[256],
                    unsigned char green[256],
                    unsigned char blue[256])
{

}

qboolean GLimp_ChangeMode( int mode )
{
    qboolean result;
    int oldvalue = r_mode->integer;

    Com_Printf("*** GLimp_ChangeMode\n");
    r_mode->integer = mode;
    if (!(result = GLimp_SetMode(qfalse)))
        r_mode->integer = oldvalue;
    
   return result;
}

/*****************************************************************************/

void *qwglGetProcAddress(const char *name)
{
    NSSymbol symbol;
    char *symbolName;

    // Prepend a '_' for the Unix C symbol mangling convention
    symbolName = malloc(strlen(name) + 2);
    strcpy(symbolName + 1, name);
    symbolName[0] = '_';

    if (NSIsSymbolNameDefined(symbolName))
        symbol = NSLookupAndBindSymbol(symbolName);
    else
        symbol = NULL;
    
    free(symbolName);
    
    if (!symbol)
        // shouldn't happen ...
        return NULL;

    return NSAddressOfSymbol(symbol);
}

/*
** GLW_InitExtensions
*/
static void GLW_InitExtensions( void )
{
        if ( !r_allowExtensions->integer )
        {
                ri.Printf( PRINT_ALL, "*** IGNORING OPENGL EXTENSIONS ***\n" );
                return;
        }

        ri.Printf( PRINT_ALL, "Initializing OpenGL extensions\n" );
        ri.Printf( PRINT_ALL, "... Supported extensions are %s\n", glConfig.extensions_string);
        
        // GL_S3_s3tc
        glConfig.textureCompression = TC_NONE;
        if ( strstr( glConfig.extensions_string, "GL_S3_s3tc" ) )
        {
                if ( r_ext_compressed_textures->integer )
                {
                        glConfig.textureCompression = TC_S3TC;
                        ri.Printf( PRINT_ALL, "...using GL_S3_s3tc\n" );
                }
                else
                {
                        glConfig.textureCompression = TC_NONE;
                        ri.Printf( PRINT_ALL, "...ignoring GL_S3_s3tc\n" );
                }
        }
        else
        {
                ri.Printf( PRINT_ALL, "...GL_S3_s3tc not found\n" );
        }


#ifdef GL_EXT_texture_env_add
        // GL_EXT_texture_env_add
        glConfig.textureEnvAddAvailable = qfalse;
        if ( strstr( glConfig.extensions_string, "GL_EXT_texture_env_add" ) )
        {
                if ( r_ext_texture_env_add->integer )
                {
                        glConfig.textureEnvAddAvailable = qtrue;
                        ri.Printf( PRINT_ALL, "...using GL_EXT_texture_env_add\n" );
                }
                else
                {
                        glConfig.textureEnvAddAvailable = qfalse;
                        ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_env_add\n" );
                }
        }
        else
        {
                ri.Printf( PRINT_ALL, "...GL_EXT_texture_env_add not found\n" );
        }
#endif

#ifdef GL_ARB_texture_env_add
        // GL_ARB_texture_env_add -- only if we didn't find GL_EXT_texture_env_add
        if (!glConfig.textureEnvAddAvailable) {
            if ( strstr( glConfig.extensions_string, "GL_ARB_texture_env_add" ) )
            {
                    if ( r_ext_texture_env_add->integer )
                    {
                            glConfig.textureEnvAddAvailable = qtrue;
                            ri.Printf( PRINT_ALL, "...using GL_ARB_texture_env_add\n" );
                    }
                    else
                    {
                            glConfig.textureEnvAddAvailable = qfalse;
                            ri.Printf( PRINT_ALL, "...ignoring GL_ARB_texture_env_add\n" );
                    }
            }
            else
            {
                    ri.Printf( PRINT_ALL, "...GL_ARB_texture_env_add not found\n" );
            }
        }
#endif


#if 0   // Win32 does this differently than we do -- I'll provide a C function that looks the same
        // that will do the correct ObjC stuff
        // WGL_EXT_swap_control
        qwglSwapIntervalEXT = ( BOOL (WINAPI *)(int)) qwglGetProcAddress( "wglSwapIntervalEXT" );
        if ( qwglSwapIntervalEXT )
        {
                ri.Printf( PRINT_ALL, "...using WGL_EXT_swap_control\n" );
                r_swapInterval->modified = qtrue;	// force a set next frame
        }
        else
        {
                ri.Printf( PRINT_ALL, "...WGL_EXT_swap_control not found\n" );
        }
#else
        if (r_swapInterval) {
            ri.Printf( PRINT_ALL, "...using +[NSOpenGLContext setParameter:] for qwglSwapIntervalEXT\n" );
            r_swapInterval->modified = qtrue;	// force a set next frame
        }
#endif
        
        // GL_ARB_multitexture
        qglMultiTexCoord2fARB = NULL;
        qglActiveTextureARB = NULL;
        qglClientActiveTextureARB = NULL;
        if ( strstr( glConfig.extensions_string, "GL_ARB_multitexture" )  )
        {
                if ( r_ext_multitexture->integer )
                {
                        qglMultiTexCoord2fARB = ( PFNGLMULTITEXCOORD2FARBPROC ) qwglGetProcAddress( "glMultiTexCoord2fARB" );
                        qglActiveTextureARB = ( PFNGLACTIVETEXTUREARBPROC ) qwglGetProcAddress( "glActiveTextureARB" );
                        qglClientActiveTextureARB = ( PFNGLCLIENTACTIVETEXTUREARBPROC ) qwglGetProcAddress( "glClientActiveTextureARB" );

                        if ( qglActiveTextureARB )
                        {
                                qglGetIntegerv( GL_MAX_ACTIVE_TEXTURES_ARB, (GLint *)&glConfig.maxActiveTextures );

                                if ( glConfig.maxActiveTextures > 1 )
                                {
                                        ri.Printf( PRINT_ALL, "...using GL_ARB_multitexture\n" );
                                }
                                else
                                {
                                        qglMultiTexCoord2fARB = NULL;
                                        qglActiveTextureARB = NULL;
                                        qglClientActiveTextureARB = NULL;
                                        ri.Printf( PRINT_ALL, "...not using GL_ARB_multitexture, < 2 texture units\n" );
                                }
                        }
                }
                else
                {
                        ri.Printf( PRINT_ALL, "...ignoring GL_ARB_multitexture\n" );
                }
        }
        else
        {
                ri.Printf( PRINT_ALL, "...GL_ARB_multitexture not found\n" );
        }

        // GL_EXT_compiled_vertex_array
        qglLockArraysEXT = NULL;
        qglUnlockArraysEXT = NULL;
        if ( strstr( glConfig.extensions_string, "GL_EXT_compiled_vertex_array" ) )
        {
                if ( r_ext_compiled_vertex_array->integer )
                {
                        ri.Printf( PRINT_ALL, "...using GL_EXT_compiled_vertex_array\n" );
                        qglLockArraysEXT = ( void ( APIENTRY * )( GLint, GLint ) ) qwglGetProcAddress( "glLockArraysEXT" );
                        qglUnlockArraysEXT = ( void ( APIENTRY * )( void ) ) qwglGetProcAddress( "glUnlockArraysEXT" );
                        if (!qglLockArraysEXT || !qglUnlockArraysEXT) {
                                ri.Error (ERR_FATAL, "bad getprocaddress\n");
                        }
                }
                else
                {
                        ri.Printf( PRINT_ALL, "...ignoring GL_EXT_compiled_vertex_array\n" );
                }
        }
        else
        {
                ri.Printf( PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n" );
        }

#ifdef GL_APPLE_transform_hint
        if ( strstr( glConfig.extensions_string, "GL_APPLE_transform_hint" )  ) {
            r_appleTransformHint = ri.Cvar_Get("r_appleTransformHint", "1", CVAR_ARCHIVE );
            if (r_appleTransformHint->value) {
                ri.Printf( PRINT_ALL, "...using GL_APPLE_transform_hint\n");
                qglHint(GL_TRANSFORM_HINT_APPLE, GL_FASTEST);
                CheckErrors();
            } else {
                ri.Printf( PRINT_ALL, "...ignoring using GL_APPLE_transform_hint\n");
            }
        } else {
            ri.Printf( PRINT_ALL, "...GL_APPLE_transform_hint not found\n" );
        }
#endif
}


#define MAX_RENDERER_INFO_COUNT 128

// Returns zero if there are no hardware renderers.  Otherwise, returns the max memory across all renderers (on the presumption that the screen that we'll use has the most memory).
static GLint Sys_QueryVideoMemory()
{
    CGLError err;
    CGLRendererInfoObj rendererInfo, rendererInfos[MAX_RENDERER_INFO_COUNT];
    GLint rendererInfoIndex, rendererInfoCount = MAX_RENDERER_INFO_COUNT;
    GLint rendererIndex = 0, rendererCount;
    GLint maxVRAM = 0, vram = 0;
    GLint accelerated;
    GLint rendererID;
    GLint totalRenderers = 0;
    
    err = CGLQueryRendererInfo(CGDisplayIDToOpenGLDisplayMask(Sys_DisplayToUse()), rendererInfos, &rendererInfoCount);
    if (err) {
        Com_Printf("CGLQueryRendererInfo -> %d\n", err);
        return vram;
    }
    
    //Com_Printf("rendererInfoCount = %d\n", rendererInfoCount);
    for (rendererInfoIndex = 0; rendererInfoIndex < rendererInfoCount-1/* && totalRenderers < rendererInfoCount*/; rendererInfoIndex++) {
        rendererInfo = rendererInfos[rendererInfoIndex];
        //Com_Printf("rendererInfo: 0x%08x\n", rendererInfo);
        

        err = CGLDescribeRenderer(rendererInfo, 0, kCGLRPRendererCount, &rendererCount);
        if (err) {
            Com_Printf("CGLDescribeRenderer(kCGLRPRendererID) -> %d\n", err);
            continue;
        }
        //Com_Printf("  rendererCount: %d\n", rendererCount);

        
            totalRenderers++;
            //Com_Printf("  rendererIndex: %d\n", rendererIndex);
            
            rendererID = 0xffffffff;
            err = CGLDescribeRenderer(rendererInfo, rendererIndex, kCGLRPRendererID, &rendererID);
            if (err) {
                Com_Printf("CGLDescribeRenderer(kCGLRPRendererID) -> %d\n", err);
                continue;
            }
            //Com_Printf("    rendererID: 0x%08x\n", rendererID);
            
            accelerated = 0;
            err = CGLDescribeRenderer(rendererInfo, rendererIndex, kCGLRPAccelerated, &accelerated);
            if (err) {
                Com_Printf("CGLDescribeRenderer(kCGLRPAccelerated) -> %d\n", err);
                continue;
            }
            //Com_Printf("    accelerated: %d\n", accelerated);
            if (!accelerated)
                continue;
            
            vram = 0;
            err = CGLDescribeRenderer(rendererInfo, rendererIndex, kCGLRPVideoMemory, &vram);
            if (err) {
                Com_Printf("CGLDescribeRenderer -> %d\n", err);
                continue;
            }
            //Com_Printf("    vram: 0x%08x\n", vram);
            
            // presumably we'll be running on the best card, so we'll take the max of the vrams
            if (vram > maxVRAM)
                maxVRAM = vram;
        
        
#if 0
        err = CGLDestroyRendererInfo(rendererInfo);
        if (err) {
            Com_Printf("CGLDestroyRendererInfo -> %d\n", err);
        }
#endif
    }

    return maxVRAM;
}


// We will set the Sys_IsHidden global to cause input to be handle differently (we'll just let NSApp handle events in this case).  We also will unbind the GL context and restore the video mode.
qboolean Sys_Hide()
{
    if (Sys_IsHidden)
        // Eh?
        return qfalse;
    
    if (!r_fullscreen->integer)
        // We only support hiding in fullscreen mode right now
        return qfalse;
    
    Sys_IsHidden = qtrue;

    // Disassociate the GL context from the screen
    // Have to call both to actually deallocate kernel resources and free the NSSurface
    CGLClearDrawable(OSX_GetCGLContext());
    [OSX_GetNSGLContext() clearDrawable];
    
    // Restore the original video mode
    _GLimp_RestoreOriginalVideoSettings();

    // Restore the original gamma if needed.
    if (glConfig.deviceSupportsGamma) {
        CGDisplayRestoreColorSyncSettings();
    }

    // Release the screen(s)
    ReleaseAllDisplays();
    

    // Shut down the input system so the mouse and keyboard settings are restore to normal
    Sys_ShutdownInput();
    
    // Hide the application so that when the user clicks on our app icon, we'll get an unhide notification
    [NSApp hide: nil];
    
    return qtrue;
}

static CGDisplayErr Sys_CaptureActiveDisplays(void)
{
    CGDisplayErr err;
    CGDisplayCount displayIndex;
    for (displayIndex = 0; displayIndex < glw_state.displayCount; displayIndex++) {
	const glwgamma_t *table;
	table = &glw_state.originalDisplayGammaTables[displayIndex];
	err = CGDisplayCapture(table->display);
	if (err != CGDisplayNoErr)
	    return err;
    }
    return CGDisplayNoErr;
}

qboolean Sys_Unhide()
{
    CGDisplayErr err;
    CGLError glErr;
    
    if (!Sys_IsHidden)
        // Eh?
        return qfalse;
    
    // Capture the screen(s)
    err = Sys_CaptureActiveDisplays();
    if (err != CGDisplayNoErr) {
        ri.Printf( PRINT_ALL, "Unhide failed -- cannot capture the display again.\n" );
        return qfalse;
    }
    
    // Restore the game mode
    err = CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.gameMode);
    if ( err != CGDisplayNoErr ) {
        ReleaseAllDisplays();
        ri.Printf( PRINT_ALL, "Unhide failed -- Unable to set display mode\n" );
        return qfalse;
    }

    // Reassociate the GL context and the screen
    glErr = CGLSetFullScreen(OSX_GetCGLContext());
    if (err) {
        ReleaseAllDisplays();
        ri.Printf( PRINT_ALL, "Unhide failed: CGLSetFullScreen -> %d (%s)\n", err, CGLErrorString(err));
        return qfalse;
    }

    // Restore the current context
    [OSX_GetNSGLContext() makeCurrentContext];
    
    // Restore the input system (last so if something goes wrong we don't eat the mouse)
    Sys_InitInput();
    
    Sys_IsHidden = qfalse;
    return qtrue;
}


