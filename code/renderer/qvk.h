
/*
** QVK.H
*/

#ifndef __QVK_H__
#define __QVK_H__

#if defined( _WIN32 )

#pragma warning (disable: 4201)
#pragma warning (disable: 4214)
#pragma warning (disable: 4514)
#pragma warning (disable: 4032)
#pragma warning (disable: 4201)
#pragma warning (disable: 4214)
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR 
#define VK_NO_PROTOTYPES 
#include <vulkan/vulkan.h>

#elif defined(__APPLE__)

#include "macosx_glimp.h" // "../macosx/macosx_glimp.h"

#elif defined( __linux__ )

#else

#include <gl.h>

#endif

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef WINAPI
#define WINAPI
#endif


PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

// GLOBAL
PFN_vkCreateInstance vkCreateInstance;
PFN_vkEnumerateInstanceExtensionProperties;

// INSTANCE
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
#if defined( _WIN32 )
PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
#endif

#endif