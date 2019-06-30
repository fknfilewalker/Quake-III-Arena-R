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
/*
** QVK_WIN.C
**
** This file implements the operating system binding of GL to QGL function
** pointers.  When doing a port of Quake3 you must implement the following
** two functions:
**
** QVK_Init() - loads libraries, assigns function pointers, etc.
** QVK_Shutdown() - unloads libraries, NULLs function pointers
*/
#include <float.h>
#include "../renderer/tr_local.h"
#include "glw_win.h"

//
//#define VK_EXPORTED_FUNCTION( fun )                                                   \
//    if( !(fun = (PFN_##fun)GetProcAddress( glw_state.hinstOpenGL, #fun )) ) {                \
//		ri.Printf(PRINT_ALL, "Could not load exported function: %s !", #fun);  \
//		return qfalse;                                                                   \
//    }

#define VK_EXPORTED_FUNCTION( a ) GetProcAddress( glw_state.hinstVulkan, a )
#define VK_GLOBAL_LEVEL_FUNCTION( a ) vkGetInstanceProcAddr( NULL, a )
#define VK_INSTANCE_LEVEL_FUNCTION( a ) vkGetInstanceProcAddr( vkInstance.instance , a)

//#define VK_GLOBAL_LEVEL_FUNCTION( fun )                                                   \
//    if( !(fun = (PFN_##fun)vkGetInstanceProcAddr( nullptr, #fun )) ) {                    \
//      std::cout << "Could not load global level function: " << #fun << "!" << std::endl;  \
//      return false;                                                                       \
//    }
//#define VK_INSTANCE_LEVEL_FUNCTION( fun )                                                   \
//    if( !(fun = (PFN_##fun)vkGetInstanceProcAddr( Vulkan.Instance, #fun )) ) {              \
//      std::cout << "Could not load instance level function: " << #fun << "!" << std::endl;  \
//      return false;                                                                         \
//    }
//#define VK_DEVICE_LEVEL_FUNCTION( fun )                                                   \
//    if( !(fun = (PFN_##fun)vkGetDeviceProcAddr( Vulkan.Device, #fun )) ) {                \
//      std::cout << "Could not load device level function: " << #fun << "!" << std::endl;  \
//      return false;                                                                       \
//    }

//VK_EXPORTED_FUNCTION(vkGetInstanceProcAddr)
//PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

//PFN_vkVoidFunction (WINAPI* qwvkGetInstanceProcAddr)(
//	VkInstance instance,
//	const char* pName);

//VkResult(WINAPI* qwvkCreateInstance)(	const VkInstanceCreateInfo* pCreateInfo,
//									const VkAllocationCallbacks* pAllocator,
//									VkInstance* pInstance);

//#define INIT_INSTANCE_FUNCTION(func) func = (PFN_ ## func)vkGetInstanceProcAddr(vk.instance, #func);
//#define INIT_DEVICE_FUNCTION(func) func = (PFN_ ## func)vkGetDeviceProcAddr(vk.device, #func);

//#define GPA( a ) GetProcAddress( glw_state.hinstVulkan, a )


/*
** QVK_Init
*/
qboolean QVK_Init(const char* dllname)
{
	char systemDir[1024];
	char libName[1024];

	GetSystemDirectory(systemDir, sizeof(systemDir));

	assert(glw_state.hinstVulkan == 0);

	ri.Printf(PRINT_ALL, "...initializing QVK\n");

	// NOTE: this assumes that 'dllname' is lower case (and it should be)!
	if (dllname[0] != '!')
	{
		Com_sprintf(libName, sizeof(libName), "%s\\%s", systemDir, dllname);
	}
	else
	{
		Q_strncpyz(libName, dllname, sizeof(libName));
	}

	ri.Printf(PRINT_ALL, "...calling LoadLibrary( '%s.dll' ): ", libName);

	if ((glw_state.hinstVulkan = LoadLibrary(dllname)) == 0)
	{
		ri.Printf(PRINT_ALL, "failed\n");
		return qfalse;
	}
	ri.Printf(PRINT_ALL, "succeeded\n");

	// Load Functions
	vkGetInstanceProcAddr = VK_EXPORTED_FUNCTION("vkGetInstanceProcAddr");

	// Global Functions
	vkCreateInstance = VK_GLOBAL_LEVEL_FUNCTION("vkCreateInstance");
	vkEnumerateInstanceExtensionProperties = VK_GLOBAL_LEVEL_FUNCTION("vkEnumerateInstanceExtensionProperties");

	return qtrue;
}

qboolean QVK_LoadInstanceFunctions(void)
{
	vkCreateWin32SurfaceKHR = VK_INSTANCE_LEVEL_FUNCTION("vkCreateWin32SurfaceKHR");
	vkEnumeratePhysicalDevices = VK_INSTANCE_LEVEL_FUNCTION("vkEnumeratePhysicalDevices");
	vkEnumerateDeviceExtensionProperties = VK_INSTANCE_LEVEL_FUNCTION("vkEnumerateDeviceExtensionProperties");
	vkGetPhysicalDeviceProperties = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceProperties");
	vkGetPhysicalDeviceSurfaceSupportKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfaceSupportKHR");
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	vkGetPhysicalDeviceSurfaceSupportKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfaceSupportKHR");
	vkGetPhysicalDeviceQueueFamilyProperties = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceQueueFamilyProperties");
	vkGetPhysicalDeviceSurfacePresentModesKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfacePresentModesKHR");
	vkGetPhysicalDeviceSurfaceFormatsKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfaceFormatsKHR");
	return qtrue;
}

/*
** QVK_Shutdown
**
** Unloads the specified DLL then nulls out all the proc pointers.  This
** is only called during a hard shutdown of the OGL subsystem (e.g. vid_restart).
*/
void QVK_Shutdown(void)
{
	ri.Printf(PRINT_ALL, "...shutting down QVK\n");

	if (glw_state.hinstVulkan)
	{
		ri.Printf(PRINT_ALL, "...unloading Vulkan DLL\n");
		FreeLibrary(glw_state.hinstVulkan);
	}

	glw_state.hinstVulkan = NULL;

	//qwvkCreateInstance = NULL;
}