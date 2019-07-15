
#include "tr_local.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#import <mach-o/dyld.h>
#import <mach/mach.h>
#import <mach/mach_error.h>
#include <dlfcn.h>

cvar_t  *r_enablerender;                // Enable actual rendering

NSWindow *_window; // keep window handle so we don't need to recreate a window all the time

/*
 =================
 CreateGameWindow
 =================
 */
static qboolean CreateGameWindow( qboolean isSecondTry )
{
    const char *windowed[] = { "Windowed", "Fullscreen" };
    int            current_mode;
    NSOpenGLPixelFormatAttribute *pixelAttributes;
    NSOpenGLPixelFormat *pixelFormat;
    CGDisplayErr err;
    
    
    // get mode info
    current_mode = r_mode->integer;
    glConfig.isFullscreen = (r_fullscreen->integer != 0);
//
//    glw_state.desktopMode = (NSDictionary *)CGDisplayCurrentMode(glw_state.display);
//    if (!glw_state.desktopMode) {
//        ri.Error(ERR_FATAL, "Could not get current graphics mode for display 0x%08x\n", glw_state.display);
//    }
//    
//#if 0
//    ri.Printf( PRINT_ALL, "... desktop mode %d = %dx%d %s\n", glw_state.desktopMode,
//              glw_state.desktopDesc.width, glw_state.desktopDesc.height,
//              depthStrings[glw_state.desktopDesc.depth]);
//#endif
//    
    ri.Printf( PRINT_ALL, "...setting mode %d:\n", current_mode );
    if ( !R_GetModeInfo( &glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect, current_mode ) )  {
        ri.Printf( PRINT_ALL, " invalid mode\n" );
        return qfalse;
    }
    ri.Printf( PRINT_ALL, " %d %d %s\n", glConfig.vidWidth, glConfig.vidHeight, windowed[glConfig.isFullscreen] );
//
//    glw_state.gameMode = glw_state.desktopMode;
//    
//    
//    
//    // Get the GL pixel format
//    pixelAttributes = GetPixelAttributes();
//    pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes: pixelAttributes] autorelease];
//    NSZoneFree(NULL, pixelAttributes);
//    
//    if (!pixelFormat) {
//        CGDisplayRestoreColorSyncSettings();
//        CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.desktopMode);
//        ReleaseAllDisplays();
//        ri.Printf( PRINT_ALL, " No pixel format found\n");
//        return qfalse;
//    }
//    
//    // Create a context with the desired pixel attributes
//    OSX_SetGLContext([[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext: nil]);
//    if (!OSX_GetNSGLContext()) {
//        CGDisplayRestoreColorSyncSettings();
//        CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.desktopMode);
//        ReleaseAllDisplays();
//        ri.Printf(PRINT_ALL, "... +[NSOpenGLContext createWithFormat:share:] failed.\n" );
//        return qfalse;
//    }
    
    //if (!glConfig.isFullscreen) {
    cvar_t        *vid_xpos;
    cvar_t        *vid_ypos;
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
    
    NSUInteger masks = [_window styleMask];
    qboolean isFullscreen = qfalse;
    if ( masks & NSFullScreenWindowMask) {
        isFullscreen = qtrue;
    }
    
    if (glConfig.isFullscreen) {
//        glw_state.gameMode = CGDisplayBestModeForParameters (kCGDirectMainDisplay, 32, glConfig.vidWidth, glConfig.vidHeight, NULL);
//        err = CGDisplaySwitchToMode(glw_state.display, glw_state.gameMode);//(CFDictionaryRef)glw_state.gameMode);
        
        [_window toggleFullScreen:nil];
        [_window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
        [_window setFrame:windowRect display:YES];
    } else {
        
//        err = CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.desktopMode);
        
        NSSize size;
        size.height = glConfig.vidHeight;
        size.width = glConfig.vidWidth;
        
        if(isFullscreen) [_window toggleFullScreen:nil];
        [_window setContentSize:size];
        [_window setCollectionBehavior:NSWindowCollectionBehaviorDefault];
        //[glw_state.window setFrame:windowRect display:YES];
    }
    
    _window.styleMask &= ~NSWindowStyleMaskResizable;
    
//    if ( err != CGDisplayNoErr ) {
//        ri.Printf( PRINT_ALL, " Unable to set display mode, err = %d\n", err );
//        return qfalse;
//    }
    
    [_window setTitle: @"Quake3"];
    
    [_window orderFront: nil];
    

    // Always get mouse moved events (if mouse support is turned off (rare)
    // the event system will filter them out.
    [_window setAcceptsMouseMovedEvents: YES];
    
    // Hider cursor
    [NSCursor hide];
    //[NSMenu setMenuBarVisible:NO];
    
    // Direct the context to draw in this window
    //[OSX_GetNSGLContext() setView: [_window contentView]];

    // Export Function
    void *libHandle = dlopen("libvulkan.1.dylib", RTLD_LAZY | RTLD_LOCAL);
    if (!libHandle)
    {
        ri.Printf( PRINT_ERROR, " Could not load libvulkan.1.dylib");
        return qfalse;
    }
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) dlsym(libHandle, "vkGetInstanceProcAddr");
    dlclose(libHandle);
    
    // Window: set Metal Layer
    NSBundle* bundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/QuartzCore.framework"];
    if (!bundle)
    {
        ri.Printf( PRINT_ERROR, " Could not load Metal Layer");
    }
    id metalLayer = [[bundle classNamed:@"CAMetalLayer"] layer];
    
    //NSView *view = [[NSView alloc] initWithFrame:windowRect];
    //[_window setContentView:view];
    //[_window makeFirstResponder:view];
    //[view setWantsLayer:YES];
    //[a setContentsScale:[_window backingScaleFactor]];
    //[view setLayer:a];
    
    [[_window contentView] setWantsLayer:YES];
    [[_window contentView] setLayer:metalLayer];
    
    // Setup Vulkan
    VK_Setup((void*) [_window contentView], NULL);
    
    // Sync input rect with where the window actually is...
    //Sys_UpdateWindowMouseInputRect();
    
    
#ifndef USE_CGLMACROS
    // Make this the current context
    //OSX_GLContextSetCurrent();
#endif
    
//    // Store off the pixel format attributes that we actually got
//    [pixelFormat getValues: (GLint *) &glConfig.colorBits forAttribute: NSOpenGLPFAColorSize forVirtualScreen: 0];
//    [pixelFormat getValues: (GLint *) &glConfig.depthBits forAttribute: NSOpenGLPFADepthSize forVirtualScreen: 0];
//    [pixelFormat getValues: (GLint *) &glConfig.stencilBits forAttribute: NSOpenGLPFAStencilSize forVirtualScreen: 0];
//
//    glConfig.displayFrequency = [[glw_state.gameMode objectForKey: (id)kCGDisplayRefreshRate] intValue];
//
    
    ri.Printf(PRINT_ALL, "ok\n" );
    
    if (glConfig.isFullscreen) {
        [_window toggleFullScreen:nil];
        
    }
    return qtrue;
}

/*
 ** VKimp_SetMode
 */
qboolean VKimp_SetMode( qboolean isSecondTry )
{
    if ( !CreateGameWindow(isSecondTry) ) {
        ri.Printf( PRINT_ALL, "VKimp_Init: window could not be created!\n" );
        return qfalse;
    }
    
    vkattribbuffer_t a;
    VK_CreateVertexBuffer(&a, 15 * sizeof(Float32));
                    // vec3 vec4 vec2
//    float vdata[] = {0, 0, 0.1,
//                     1, 0, 0.1,
//                     1, 1, 0.1};
    Float32 vdata[] = {0.0, -0.9, 0.0,      1.0f, 1.0f, // top
        0.9, 0.9, 0.0,                      1.0f, 0.0f, // right
        -0.9, 0.9, 0.0,                     1.0f, 0.0f}; // left
    VK_UploadAttribData(&a, (void *) &vdata[0]);
    
    vkshader_t s = {0};
    VK_SingleTextureShader(&s);
    
    vkimage_t image = { 0 };
    VK_CreateImage(&image, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, 1);
    uint8_t data[4] = {0,0,255,255
    };
    Float32 data2[4] = {1,1,0,1
    };
    VK_UploadImageData(&image, 1, 1, &data, 4, 0); // rows wise
    //VK_UploadImageData(&image, 1, 1, &data2, 16, 1);
    VK_CreateSampler(&image, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    
    
    vkdescriptor_t d = {0};
    VK_AddSampler(&d, 0, VK_SHADER_STAGE_FRAGMENT_BIT);
    //VK_AddSampler(&d, 1, VK_SHADER_STAGE_VERTEX_BIT);
    VK_SetSampler(&d, 0, VK_SHADER_STAGE_FRAGMENT_BIT, image.sampler, image.view);
    VK_FinishDescriptor(&d);
    
    
    vkpipeline_t p = {0};
    VK_SetDescriptorSet(&p, &d);
    VK_SetShader(&p, &s);
    VK_AddBindingDescription(&p, 0, 5 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX);
    VK_AddAttributeDescription(&p, 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 * sizeof(float));
    VK_AddAttributeDescription(&p, 1, 0, VK_FORMAT_R32G32_SFLOAT, 3 * sizeof(float));
    VK_FinishPipeline(&p);

    // draw something to show that VK is alive
    if (r_enablerender->integer) {
        VK_BeginFrame();
        beginRenderClear();
        VK_Draw(&p, &a, 3);
        endRender();
        VK_EndFrame();

        VK_BeginFrame();
        beginRenderClear();
        VK_Draw(&p, &a, 3);
        endRender();
        VK_EndFrame();
        
        VK_BeginFrame();
        beginRenderClear();
        VK_Draw(&p, &a, 3);
        endRender();
        VK_EndFrame();
        
        VK_BeginFrame();
        beginRenderClear();
        VK_Draw(&p, &a, 3);
        endRender();
        VK_EndFrame();
        
        VK_BeginFrame();
        beginRenderClear();
        VK_Draw(&p, &a, 3);
        endRender();
        VK_EndFrame();
    }
    
    return qtrue;
}

void VKimp_Init( void )
{
    cvar_t *lastValidRenderer = ri.Cvar_Get( "r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE );
    char *buf;
    
    ri.Printf( PRINT_ALL, "Initializing Vulkan subsystem\n" );
    ri.Printf( PRINT_ALL, "  Last renderer was '%s'\n", lastValidRenderer->string);
    ri.Printf( PRINT_ALL, "  r_fullscreen = %d\n", r_fullscreen->integer);
    
    memset( &glConfig, 0, sizeof( glConfig ) );
    
    // We only allow changing the gamma if we are full screen
    glConfig.deviceSupportsGamma = false;//(r_fullscreen->integer != 0);
    
    r_enablerender = ri.Cvar_Get("r_enablerender", "1", 0 );
    
    if ( ! VKimp_SetMode(qfalse) ) {
        // fall back to the known-good mode
        ri.Cvar_Set( "r_fullscreen", "1" );
        ri.Cvar_Set( "r_mode", "3" );
        ri.Cvar_Set( "r_stereo", "0" );
        ri.Cvar_Set( "r_depthBits", "16" );
        ri.Cvar_Set( "r_colorBits", "16" );
        ri.Cvar_Set( "r_stencilBits", "0" );
        if ( VKimp_SetMode(qtrue) ) {
            ri.Printf( PRINT_ALL, "------------------\n" );
            return;
        }
        
        ri.Error( ERR_FATAL, "Could not initialize Vulkan\n" );
        return;
    }

    ri.Printf( PRINT_ALL, "------------------\n" );
//    
//    // get our config strings
//    Q_strncpyz( glConfig.vendor_string, (const char *)qglGetString (GL_VENDOR), sizeof( glConfig.vendor_string ) );
//    Q_strncpyz( glConfig.renderer_string, (const char *)qglGetString (GL_RENDERER), sizeof( glConfig.renderer_string ) );
//    Q_strncpyz( glConfig.version_string, (const char *)qglGetString (GL_VERSION), sizeof( glConfig.version_string ) );
//    Q_strncpyz( glConfig.extensions_string, (const char *)qglGetString (GL_EXTENSIONS), sizeof( glConfig.extensions_string ) );
//    
//    //
//    // chipset specific configuration
//    //
//    buf = malloc(strlen(glConfig.renderer_string) + 1);
//    strcpy( buf, glConfig.renderer_string );
//    Q_strlwr( buf );
//    
//    ri.Cvar_Set( "r_lastValidRenderer", glConfig.renderer_string );
//    free(buf);
//    
//    GLW_InitExtensions();
//    
//#ifndef USE_CGLMACROS
//    if (!r_enablerender->integer)
//        OSX_GLContextClearCurrent();
//#endif
//
}


