
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

#define VK_USE_PLATFORM_MACOS_MVK
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

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


PFN_vkGetInstanceProcAddr						vkGetInstanceProcAddr;

/*
** GLOBAL
*/
PFN_vkCreateInstance							vkCreateInstance;
PFN_vkEnumerateInstanceExtensionProperties		vkEnumerateInstanceExtensionProperties;
PFN_vkEnumerateInstanceLayerProperties			vkEnumerateInstanceLayerProperties;

/*
** INSTANCE
*/
PFN_vkEnumeratePhysicalDevices					vkEnumeratePhysicalDevices;
PFN_vkEnumerateDeviceExtensionProperties		vkEnumerateDeviceExtensionProperties;
PFN_vkGetPhysicalDeviceMemoryProperties			vkGetPhysicalDeviceMemoryProperties;

/* Surface */
#if defined( _WIN32 )
PFN_vkCreateWin32SurfaceKHR						vkCreateWin32SurfaceKHR;
#elif defined(__APPLE__)
PFN_vkCreateMacOSSurfaceMVK                     vkCreateMacOSSurfaceMVK;
#elif defined( __linux__ )

#endif

/* Physical Device */
PFN_vkGetPhysicalDeviceProperties				vkGetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR		vkGetPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR	vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR		vkGetPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetPhysicalDeviceQueueFamilyProperties	vkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR	vkGetPhysicalDeviceSurfacePresentModesKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR		vkGetPhysicalDeviceSurfaceFormatsKHR;

/* Device */
PFN_vkGetDeviceProcAddr							vkGetDeviceProcAddr;
PFN_vkCreateDevice								vkCreateDevice;

/*
** DEVICE
*/
PFN_vkGetDeviceQueue							vkGetDeviceQueue;
PFN_vkCreateCommandPool							vkCreateCommandPool;
PFN_vkCreateSwapchainKHR						vkCreateSwapchainKHR;
PFN_vkGetSwapchainImagesKHR						vkGetSwapchainImagesKHR;
PFN_vkCreateImageView							vkCreateImageView;
PFN_vkCreateSampler								vkCreateSampler;
PFN_vkCreateRenderPass							vkCreateRenderPass;
PFN_vkCreateFramebuffer							vkCreateFramebuffer;
PFN_vkAllocateCommandBuffers					vkAllocateCommandBuffers;
PFN_vkCreateSemaphore							vkCreateSemaphore;
PFN_vkCreateFence								vkCreateFence;

PFN_vkWaitForFences								vkWaitForFences;
PFN_vkResetFences								vkResetFences;
PFN_vkAcquireNextImageKHR						vkAcquireNextImageKHR;
PFN_vkFreeCommandBuffers						vkFreeCommandBuffers;
PFN_vkBeginCommandBuffer						vkBeginCommandBuffer;

PFN_vkEndCommandBuffer							vkEndCommandBuffer;
PFN_vkQueueSubmit								vkQueueSubmit;
PFN_vkQueueWaitIdle								vkQueueWaitIdle;
PFN_vkQueuePresentKHR							vkQueuePresentKHR;

PFN_vkCmdBeginRenderPass						vkCmdBeginRenderPass;
PFN_vkCmdSetViewport							vkCmdSetViewport;
PFN_vkCmdSetScissor								vkCmdSetScissor;
PFN_vkCmdEndRenderPass							vkCmdEndRenderPass;
PFN_vkCmdBindVertexBuffers                      vkCmdBindVertexBuffers;
PFN_vkCmdBindIndexBuffer                        vkCmdBindIndexBuffer;
PFN_vkCmdPushConstants                          vkCmdPushConstants;
PFN_vkCmdClearAttachments                       vkCmdClearAttachments;

PFN_vkCreateImage								vkCreateImage;
PFN_vkGetImageMemoryRequirements				vkGetImageMemoryRequirements;
PFN_vkGetBufferMemoryRequirements				vkGetBufferMemoryRequirements;

PFN_vkCreateBuffer								vkCreateBuffer;
PFN_vkAllocateMemory							vkAllocateMemory;
PFN_vkBindBufferMemory							vkBindBufferMemory;
PFN_vkBindImageMemory							vkBindImageMemory;
PFN_vkMapMemory									vkMapMemory;
PFN_vkUnmapMemory								vkUnmapMemory;

PFN_vkDestroyBuffer								vkDestroyBuffer;
PFN_vkFreeMemory								vkFreeMemory;

PFN_vkAllocateCommandBuffers					vkAllocateCommandBuffers;
PFN_vkBeginCommandBuffer						vkBeginCommandBuffer;
PFN_vkEndCommandBuffer							vkEndCommandBuffer;
PFN_vkFreeCommandBuffers						vkFreeCommandBuffers;

PFN_vkCmdPipelineBarrier						vkCmdPipelineBarrier;
PFN_vkCmdCopyBufferToImage						vkCmdCopyBufferToImage;
PFN_vkCmdBindPipeline                           vkCmdBindPipeline;
PFN_vkCmdBindDescriptorSets                     vkCmdBindDescriptorSets;
PFN_vkCmdBindVertexBuffers                      vkCmdBindVertexBuffers;
PFN_vkCmdDraw                                   vkCmdDraw;
PFN_vkCmdDrawIndexed                            vkCmdDrawIndexed;
PFN_vkCmdPushConstants                          vkCmdPushConstants;

PFN_vkCreatePipelineCache                       vkCreatePipelineCache;
PFN_vkCreatePipelineLayout                      vkCreatePipelineLayout;
PFN_vkCreateGraphicsPipelines                   vkCreateGraphicsPipelines;

PFN_vkCreateShaderModule                        vkCreateShaderModule;
PFN_vkCreateDescriptorSetLayout                 vkCreateDescriptorSetLayout;
PFN_vkCreateDescriptorPool                      vkCreateDescriptorPool;
PFN_vkUpdateDescriptorSets                      vkUpdateDescriptorSets;

PFN_vkDestroySampler                            vkDestroySampler;

/* Debug */
#ifndef NDEBUG
PFN_vkCreateDebugUtilsMessengerEXT				vkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT				vkDestroyDebugUtilsMessengerEXT;
#endif

#endif
