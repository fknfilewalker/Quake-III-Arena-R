#include "../tr_local.h"

static void VK_AllocateMemory(vkimage_t* image, VkMemoryPropertyFlags properties);
void VK_CreateSampler(vkimage_t* image, VkFilter magFilter, VkFilter minFilter,
	VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode);
static void VK_CopyBufferToImage(vkimage_t* image, uint32_t width, uint32_t height, VkBuffer *buffer, uint32_t mipLevel);

void VK_CreateImage(vkimage_t *image, uint32_t width, uint32_t height, VkFormat format, uint32_t mipLevels) {
	image->extent = (VkExtent3D) { width, height, 1 };
	image->mipLevels = mipLevels;

	// create image
	{
		VkImageCreateInfo desc = {0};
		desc.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		desc.pNext = NULL;
		desc.flags = 0;
		desc.imageType = VK_IMAGE_TYPE_2D;
		desc.format = format;
		desc.extent.width = width;
		desc.extent.height = height;
		desc.extent.depth = 1;
		desc.mipLevels = image->mipLevels;
		desc.arrayLayers = 1;
		desc.samples = VK_SAMPLE_COUNT_1_BIT;
		desc.tiling = VK_IMAGE_TILING_OPTIMAL;
		desc.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		desc.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		desc.queueFamilyIndexCount = 0;
		desc.pQueueFamilyIndices = NULL;
		desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VK_CHECK(vkCreateImage(vk.device, &desc, NULL, &image->handle), "failed to create Image!");
		VK_AllocateMemory(image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	// create image view
	{
		VkImageViewCreateInfo desc = {0};
		desc.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		desc.pNext = NULL;
		desc.flags = 0;
		desc.image = image->handle;
		desc.viewType = VK_IMAGE_VIEW_TYPE_2D;
		desc.format = format;
		desc.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		desc.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		desc.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		desc.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		desc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		desc.subresourceRange.baseMipLevel = 0;
		desc.subresourceRange.levelCount = image->mipLevels;//VK_REMAINING_MIP_LEVELS;
		desc.subresourceRange.baseArrayLayer = 0;
		desc.subresourceRange.layerCount = 1;
		VK_CHECK(vkCreateImageView(vk.device, &desc, NULL, &image->view), "failed to create Image View!");
	}
}

static void VK_AllocateMemory(vkimage_t *image, VkMemoryPropertyFlags properties) {
	VkMemoryRequirements memRequirements = {0};
	vkGetImageMemoryRequirements(vk.device, image->handle, &memRequirements);

	int32_t memoryTypeIndex = VK_FindMemoryTypeIndex(vk.physical_device, memRequirements.memoryTypeBits, properties);

	VkMemoryAllocateInfo allocInfo = {0};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memoryTypeIndex != -1 ? memoryTypeIndex : VK_DeviceLocalMemoryIndex(vk.physical_device);

	VK_CHECK(vkAllocateMemory(vk.device, &allocInfo, NULL, &image->memory), "failed to allocate Image Memory!");

	VK_CHECK(vkBindImageMemory(vk.device, image->handle, image->memory, 0), "failed to bind Image Memory!");
}

void VK_CreateSampler(	vkimage_t* image, VkFilter magFilter, VkFilter minFilter, 
						VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode)
{
    if(image->sampler != NULL) {
        vkDestroySampler(vk.device, image->sampler, NULL);
        image->sampler = VK_NULL_HANDLE;
    }
    
	VkSamplerCreateInfo desc = { 0 };
	desc.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	desc.pNext = NULL;
	desc.flags = 0;
	desc.magFilter = magFilter;
	desc.minFilter = minFilter;
	desc.mipmapMode = mipmapMode;
	desc.addressModeU = addressMode;
	desc.addressModeV = addressMode;
	desc.addressModeW = addressMode;
	desc.mipLodBias = 0.0f;
	desc.anisotropyEnable = VK_FALSE;
	desc.maxAnisotropy = 1;
	desc.compareEnable = VK_FALSE;
	desc.compareOp = VK_COMPARE_OP_ALWAYS;
	desc.minLod = 0.0f;
	desc.maxLod = 12.00f;
	desc.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	desc.unnormalizedCoordinates = VK_FALSE;
	VK_CHECK(vkCreateSampler(vk.device, &desc, NULL, &image->sampler), "failed to create Sampler!");
}

//VkImage image, int width, int height, bool mipmap, const uint8_t* pixels, int bytes_per_pixel
void VK_UploadImageData(vkimage_t* image, uint32_t width, uint32_t height, const uint8_t* pixels, uint32_t bytes_per_pixel, uint32_t mipLevel) {

	VkDeviceSize imageSize = (uint64_t) width * (uint64_t) height * (uint64_t) 1 * (uint64_t)bytes_per_pixel;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VK_CreateBuffer( imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_HostVisibleMemoryIndex(), &stagingBuffer, &stagingBufferMemory);

	// write data to buffer
	uint8_t* p;
	VK_CHECK(vkMapMemory(vk.device, stagingBufferMemory, 0, imageSize, 0, (void**)(&p)), "failed to Map Memory!");
	Com_Memcpy(p, pixels, (size_t)(imageSize));
	vkUnmapMemory(vk.device, stagingBufferMemory);

	//createImage(VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, format, textureImage, textureImageMemory);

	VK_CopyBufferToImage(image, width, height, &stagingBuffer, mipLevel);

	vkDestroyBuffer(vk.device, stagingBuffer, NULL);
	vkFreeMemory(vk.device, stagingBufferMemory, NULL);
}

static void VK_CopyBufferToImage(vkimage_t* image, uint32_t width, uint32_t height, VkBuffer *buffer, uint32_t mipLevel)
{
	VkCommandBuffer commandBuffer;
	VK_BeginSingleTimeCommands(&commandBuffer);

	VkImageMemoryBarrier barrier = { 0 };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = image->mipLevels;

	// transition to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.image = image->handle;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, NULL, 0, NULL,
		1, &barrier);

	// buffer to image

	VkBufferImageCopy region = { 0 };
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = mipLevel;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = (VkOffset3D){ 0, 0, 0 };
    region.imageExtent = (VkExtent3D){ width, height, 1 };

	vkCmdCopyBufferToImage(commandBuffer, *buffer, image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	// transition to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL

	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.image = image->handle;
	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, NULL, 0, NULL,
		1, &barrier);

	VK_EndSingleTimeCommands(&commandBuffer);
}

void VK_DestroyImage(vkimage_t* image){
	VK_CHECK(vkQueueWaitIdle(vk.graphicsQueue), "failed to wait for Queue execution!");

    if(image->handle != NULL) {
        vkDestroyImage(vk.device, image->handle, NULL);
        image->handle = VK_NULL_HANDLE;
    }
    if(image->view != NULL) {
        vkDestroyImageView(vk.device, image->view, NULL);
        image->view = VK_NULL_HANDLE;
    }
    if(image->sampler != NULL) {
        vkDestroySampler(vk.device, image->sampler, NULL);
        image->sampler = VK_NULL_HANDLE;
    }
	if (image->memory != NULL) {
		vkFreeMemory(vk.device, image->memory, NULL);
		image->memory = VK_NULL_HANDLE;
	}
    
    VK_DestroyDescriptor(&image->descriptor_set);

	memset(image, 0, sizeof(vkimage_t));
}

void VK_ReadPixelsScreen(qboolean alpha, byte* buffer) {
	vkDeviceWaitIdle(vk.device);

	// Create image to copt swapchain content on host
	VkImageCreateInfo desc = {0};
	desc.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	desc.pNext = NULL;
	desc.flags = 0;
	desc.imageType = VK_IMAGE_TYPE_2D;
	desc.format = VK_FORMAT_R8G8B8A8_UNORM;
	desc.extent.width = glConfig.vidWidth;
	desc.extent.height = glConfig.vidHeight;
	desc.extent.depth = 1;
	desc.mipLevels = 1;
	desc.arrayLayers = 1;
	desc.samples = VK_SAMPLE_COUNT_1_BIT;
	desc.tiling = VK_IMAGE_TILING_LINEAR;
	desc.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	desc.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	desc.queueFamilyIndexCount = 0;
	desc.pQueueFamilyIndices = NULL;
	desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage image;
	VK_CHECK(vkCreateImage(vk.device, &desc, NULL, &image), "failed to create image");

	VkMemoryRequirements mRequirements;
	vkGetImageMemoryRequirements(vk.device, image, &mRequirements);

	VkMemoryAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.allocationSize = mRequirements.size;
	alloc_info.memoryTypeIndex = VK_HostVisibleMemoryIndex();

	VkDeviceMemory memory;
	VK_CHECK(vkAllocateMemory(vk.device, &alloc_info, NULL, &memory), "could not allocate Image Memory");
	VK_CHECK(vkBindImageMemory(vk.device, image, memory, 0), "could not bind Image Memory");

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	VK_BeginSingleTimeCommands(&commandBuffer);

	// transition dst image
	VkImageMemoryBarrier barrier = { 0 };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;

	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.image = image;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, NULL, 0, NULL,
		1, &barrier);

	// transition swap chain image
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;

	barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.image = vk.swapchain.images[vk.swapchain.currentImage];

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, NULL, 0, NULL,
		1, &barrier);

	VkImageCopy region = { 0 };
	region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.srcSubresource.mipLevel = 0;
	region.srcSubresource.baseArrayLayer = 0;
	region.srcSubresource.layerCount = 1;
	region.dstSubresource = region.srcSubresource;
	region.dstOffset = region.srcOffset;
	region.extent.width = glConfig.vidWidth;
	region.extent.height = glConfig.vidHeight;
	region.extent.depth = 1;

	vkCmdCopyImage(commandBuffer, vk.swapchain.images[vk.swapchain.currentImage], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
	
	VK_EndSingleTimeCommands(&commandBuffer);

	// Copy data from destination image to memory buffer.
	VkImageSubresource subresource = {0};
	subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource.mipLevel = 0;
	subresource.arrayLayer = 0;
	VkSubresourceLayout layout = { 0 };
	vkGetImageSubresourceLayout(vk.device, image, &subresource, &layout);
	
	VkFormat format = vk.swapchain.imageFormat;
	qboolean swizzleComponents = (format == VK_FORMAT_B8G8R8A8_SRGB || format == VK_FORMAT_B8G8R8A8_UNORM || format == VK_FORMAT_B8G8R8A8_SNORM);

	byte* data;
	VK_CHECK(vkMapMemory(vk.device, memory, 0, VK_WHOLE_SIZE, 0, (void**)& data), "failed to map memory");
	data += layout.size - layout.rowPitch;

	byte* pBuffer = buffer;
	for (int y = 0; y < glConfig.vidHeight; y++) {
		for (int x = 0; x < glConfig.vidWidth; x++) {
			byte pixel[4];
			Com_Memcpy(&pixel, &data[x*4], 4);

			// copy pixel to buffer
			pBuffer[0] = pixel[0];
			pBuffer[1] = pixel[1];
			pBuffer[2] = pixel[2];
			if(alpha) pBuffer[3] = pixel[3];
			if (swizzleComponents) {
				pBuffer[0] = pixel[2];
				pBuffer[2] = pixel[0];
			}

			// offset buffer by 3 (RGB) or 4 (RGBA)
			pBuffer += alpha ? 4 : 3;
		}
		data -= layout.rowPitch;
	}

	vkDestroyImage(vk.device, image, NULL);
	vkFreeMemory(vk.device, memory, NULL);
}