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
    if(image->sampler) vkDestroySampler(vk.device, image->sampler, NULL);
    
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
	desc.maxLod = 0.25f;
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
