
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
	VkPhysicalDeviceProperties deviceProp;
	VK_GetDeviceProperties(&deviceProp);

	const char* vendor_name = "unknown";
	if (deviceProp.vendorID == 0x1002) {
		vendor_name = "Advanced Micro Devices, Inc.";
	}
	else if (deviceProp.vendorID == 0x10DE) {
		vendor_name = "NVIDIA Corporation";
	}
	else if (deviceProp.vendorID == 0x8086) {
		vendor_name = "Intel Corporation";
	}
	Q_strncpyz(glConfig.vendor_string, (const char*)vendor_name, sizeof(glConfig.vendor_string));
	Q_strncpyz(glConfig.renderer_string, (const char*)deviceProp.deviceName, sizeof(glConfig.renderer_string));

	uint32_t major = VK_VERSION_MAJOR(deviceProp.apiVersion);
	uint32_t minor = VK_VERSION_MINOR(deviceProp.apiVersion);
	uint32_t patch = VK_VERSION_PATCH(deviceProp.apiVersion);
	const char* version[20];
	snprintf(version, sizeof(version), "%d.%d.%d", major, minor, patch);
	Q_strncpyz(glConfig.version_string, (const char*)version, sizeof(glConfig.version_string));

	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(vk.physical_device, NULL, &extensionCount, NULL);
	VkExtensionProperties* extensions = malloc(extensionCount * sizeof(VkExtensionProperties));
	vkEnumerateDeviceExtensionProperties(vk.physical_device, NULL, &extensionCount, &extensions[0]);
	uint32_t offset = 0;
	for (uint32_t i = 0; i < extensionCount; i++) {
		Q_strncpyz(glConfig.extensions_string + offset, (const char*)extensions[i].extensionName, strlen(extensions[i].extensionName) + 1);
		offset += strlen(extensions[i].extensionName);
		Q_strncpyz(glConfig.extensions_string + offset, " ", 2);
		offset += 1;
	}
	free(extensions);

	// store surface bits
	if (vk.swapchain.imageFormat == VK_FORMAT_B8G8R8A8_UNORM) {
		glConfig.colorBits = (int)32;
	}
	if (vk.swapchain.depthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT) {
		glConfig.depthBits = (int)24;
		glConfig.stencilBits = (int)8;
	}

	glConfig.textureEnvAddAvailable = qtrue;
}

void VKimp_Shutdown(void) {
	//	const char *strings[] = { "soft", "hard" };
	const char* success[] = { "failed", "success" };
	int retVal;

	ri.Printf(PRINT_ALL, "Shutting down Vulkan subsystem\n");

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
