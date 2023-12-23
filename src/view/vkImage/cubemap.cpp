#include "cubemap.h"
#include "stb_image.h"
#include "../vkUtil/memory.h"
#include "../../control/logging.h"
#include "../vkInit/descriptors.h"

vkimage::CubeMap::CubeMap(TextureInputChunk input)
{
	logicalDevice = input.logicalDevice;
	physicalDevice = input.physicalDevice;
	filenames = input.filenames;
	commandBuffer = input.commandBuffer;
	queue = input.queue;
	layout = input.layout;
	descriptorPool = input.descriptorPool;
	load();
	ImageInputChunk imageInput;
	imageInput.logicalDevice = logicalDevice;
	imageInput.physicalDevice = physicalDevice;
	imageInput.format = vk::Format::eR8G8B8A8Unorm;
	imageInput.arrayCount = 6;
	imageInput.width = width;
	imageInput.height = height;
	imageInput.tiling = vk::ImageTiling::eOptimal;
	imageInput.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	imageInput.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	imageInput.flags = vk::ImageCreateFlagBits::eCubeCompatible;
	
	image = make_image(imageInput);
	imageMemory = make_image_memory(imageInput, image);
	populate();
	for (int i = 0; i < 6; ++i)
		free(pixels[i]);

	makeView();
	makeSampler();
	makeDescriptorSet();
}

vkimage::CubeMap::~CubeMap()
{
	logicalDevice.freeMemory(imageMemory);
	logicalDevice.destroyImage(image);
	logicalDevice.destroyImageView(imageView);
	logicalDevice.destroySampler(sampler);
}

void vkimage::CubeMap::load()
{
	for (int i = 0; i < 6; ++i)
	{
		pixels[i] = stbi_load(filenames[i], &width, &height, &channels, STBI_rgb_alpha);
		if (!pixels)
			vklogging::Logger::getLogger()->printList({ "Unable to load: ", filenames[i]});
	}
}

void vkimage::CubeMap::populate()
{
	// First create a CPU-visible buffer...
	BufferInputChunk input;
	input.logicalDevice = logicalDevice;
	input.physicalDevice = physicalDevice;
	input.memoryProperties = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
	input.usage = vk::BufferUsageFlagBits::eTransferSrc;
	size_t image_size = width * height * 4;
	input.size = image_size * 6;

	Buffer stagingBuffer = vkutil::create_buffer(input);

	// ...then fill it,
	for (int i = 0; i < 6; ++i)
	{
		void* writeLocation = logicalDevice.mapMemory(stagingBuffer.bufferMemory, image_size * i, image_size);
		memcpy(writeLocation, pixels[i], image_size);
		logicalDevice.unmapMemory(stagingBuffer.bufferMemory);
	}

	// then transfer it to image memory
	ImageLayoutTransitionJob transitionJob;
	transitionJob.commandBuffer = commandBuffer;
	transitionJob.queue = queue;
	transitionJob.image = image;
	transitionJob.oldLayout = vk::ImageLayout::eUndefined;
	transitionJob.newLayout = vk::ImageLayout::eTransferDstOptimal;
	transitionJob.arrayCount = 6;
	transition_image_layout(transitionJob);

	BufferImageCopyJob copyJob;
	copyJob.commandBuffer = commandBuffer;
	copyJob.queue = queue;
	copyJob.srcBuffer = stagingBuffer.buffer;
	copyJob.dstImage = image;
	copyJob.width = width;
	copyJob.height = height;
	copyJob.arrayCount = 6;
	copy_buffer_to_image(copyJob);

	transitionJob.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	transitionJob.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	transition_image_layout(transitionJob);

	//Now the staging buffer can be destroyed
	logicalDevice.freeMemory(stagingBuffer.bufferMemory);
	logicalDevice.destroyBuffer(stagingBuffer.buffer);
}

void vkimage::CubeMap::makeView()
{
	imageView = make_image_view(
		logicalDevice, image, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::eCube, 6
	);
}

void vkimage::CubeMap::makeSampler()
{
	// typedef struct VkSamplerCreateInfo {
	//   VkStructureType         sType;
	//   const void* pNext;
	//   VkSamplerCreateFlags    flags;
	//   VkFilter                magFilter;
	//   VkFilter                minFilter;
	//   VkSamplerMipmapMode     mipmapMode;
	//   VkSamplerAddressMode    addressModeU;
	//   VkSamplerAddressMode    addressModeV;
	//   VkSamplerAddressMode    addressModeW;
	//   float                   mipLodBias;
	//   VkBool32                anisotropyEnable;
	//   float                   maxAnisotropy;
	//   VkBool32                compareEnable;
	//   VkCompareOp             compareOp;
	//   float                   minLod;
	//   float                   maxLod;
	//   VkBorderColor           borderColor;
	//   VkBool32                unnormalizedCoordinates;
	// } VkSamplerCreateInfo;
	vk::SamplerCreateInfo samplerInfo;
	samplerInfo.flags = vk::SamplerCreateFlags();
	samplerInfo.minFilter = vk::Filter::eNearest;
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;

	samplerInfo.anisotropyEnable = false;
	samplerInfo.maxAnisotropy = 1.f;

	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = false;
	samplerInfo.compareEnable = false;
	samplerInfo.compareOp = vk::CompareOp::eAlways;

	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.mipLodBias = 0.f;
	samplerInfo.minLod = 0.f;
	samplerInfo.maxLod = 0.f;

	try
	{
		sampler = logicalDevice.createSampler(samplerInfo);
	}
	catch (vk::SystemError err)
	{
		vklogging::Logger::getLogger()->print("Failed to make sampler.");
	}

}

void vkimage::CubeMap::makeDescriptorSet()
{
	descriptorSet = vkinit::allocate_descriptor_set(logicalDevice, descriptorPool, layout);

	vk::DescriptorImageInfo imageDescriptor;
	imageDescriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	imageDescriptor.imageView = imageView;
	imageDescriptor.sampler = sampler;

	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageDescriptor;

	logicalDevice.updateDescriptorSets(descriptorWrite, nullptr);
}

void vkimage::CubeMap::use(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout)
{
	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, pipelineLayout, 1, descriptorSet, nullptr);
}