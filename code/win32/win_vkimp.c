
#include <assert.h>
#include "../renderer/tr_local.h"
#include "../qcommon/qcommon.h"
#include "resource.h"
#include "glw_win.h"
#include "win_local.h"

typedef enum {
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,

	RSERR_UNKNOWN
} rserr_t;


/*
** VKW_InitDriver
**
** - get a DC if one doesn't exist
** - create an HGLRC if one doesn't exist
*/
/*static*/ qboolean VKW_InitDriver(const char* drivername, int colorbits)
{
	VK_Setup((void*)glw_state.hinstVulkan, (void*)wv.hWnd);



	//VK_BeginFrame();
	//beginRenderClear();
	//endRender();
	//VK_EndFrame();

	//VK_BeginFrame();
	//beginRenderClear();
	//endRender();
	//VK_EndFrame();
	
	return qtrue;
};

//void VKimp_CreateSurface() {
//	VkWin32SurfaceCreateInfoKHR desc;
//	desc.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
//	desc.pNext = NULL;
//	desc.flags = 0;
//	desc.hinstance = glw_state.hinstVulkan;
//	desc.hwnd = wv.hWnd;
//	VK_CHECK(vkCreateWin32SurfaceKHR(vkInstance.instance, &desc, NULL, &vkInstance.surface));
//}

/*
** VKW_LoadVulkan
**
** GLimp_win.c internal function that attempts to load and use
** a specific OpenGL DLL.
*/
static qboolean VKW_LoadVulkan(const char* drivername)
{
	char buffer[1024];
	qboolean cdsFullscreen;

	Q_strncpyz(buffer, drivername, sizeof(buffer));
	Q_strlwr(buffer);


	if (QVK_Init(buffer))
	{
		cdsFullscreen = r_fullscreen->integer;

		// create the window and set up the context
		if (!W_StartDriverAndSetMode(drivername, r_mode->integer, r_colorbits->integer, cdsFullscreen)) goto fail;	
		
		return qtrue;
	}


fail:

	QVK_Shutdown();
	return qfalse;
}

static void VKW_StartVulkan(void) {
	//
	// load and initialize the specific Vulkan driver
	//
	if (!VKW_LoadVulkan(VULKAN_DRIVER_NAME))
	{
		ri.Error(ERR_FATAL, "VKW_StartVulkan() - could not load Vulkan subsystem\n");
	}
	
}

void VKimp_Init( void ) {

	char	buf[1024];
	cvar_t* lastValidRenderer = ri.Cvar_Get("r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE);
	cvar_t* cv;

	ri.Printf(PRINT_ALL, "Initializing Vulkan subsystem\n");

	//
	// check OS version to see if we can do fullscreen display changes
	//
	glw_state.allowdisplaydepthchange = qtrue;

	// save off hInstance and wndproc
	cv = ri.Cvar_Get("win_hinstance", "", 0);
	sscanf(cv->string, "%i", (int*)& wv.hInstance);

	cv = ri.Cvar_Get("win_wndproc", "", 0);
	sscanf(cv->string, "%p", (void**)& glw_state.wndproc);

	// load appropriate DLL and initialize subsystem
	VKW_StartVulkan();

	// get our config strings

}

void VKimp_Shutdown(void) {
	//	const char *strings[] = { "soft", "hard" };
	const char* success[] = { "failed", "success" };
	int retVal;

	ri.Printf(PRINT_ALL, "Shutting down OpenGL subsystem\n");

	// restore gamma.  We do this first because 3Dfx's extension needs a valid OGL subsystem
	WG_RestoreGamma();

	// release DC
	if (glw_state.hDC)
	{
		retVal = ReleaseDC(wv.hWnd, glw_state.hDC) != 0;
		ri.Printf(PRINT_ALL, "...releasing DC: %s\n", success[retVal]);
		glw_state.hDC = NULL;
	}

	// destroy window
	if (wv.hWnd)
	{
		ri.Printf(PRINT_ALL, "...destroying window\n");
		ShowWindow(wv.hWnd, SW_HIDE);
		DestroyWindow(wv.hWnd);
		wv.hWnd = NULL;
		glw_state.pixelFormatSet = qfalse;
	}

	// close the r_logFile
	if (glw_state.log_fp)
	{
		fclose(glw_state.log_fp);
		glw_state.log_fp = 0;
	}

	// reset display settings
	if (glw_state.cdsFullscreen)
	{
		ri.Printf(PRINT_ALL, "...resetting display\n");
		ChangeDisplaySettings(0, 0);
		glw_state.cdsFullscreen = qfalse;
	}

	// shutdown QGL subsystem
	QVK_Shutdown();

	memset(&glConfig, 0, sizeof(glConfig));
	memset(&glState, 0, sizeof(glState));
}
