#include "frame.h"
#include "memory.h"
#include "../vkImage/image.h"

void vkutil::SwapChainFrame::makeDescriptorResources()
{
	BufferInputChunk input;
	input.logicalDevice = logicalDevice;
	input.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	input.physicalDevice = physicalDevice;

	input.size = sizeof(CameraVectors);
	input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	cameraVectorBuffer = create_buffer(input);

	cameraVectorWriteLocation = logicalDevice.mapMemory(cameraVectorBuffer.bufferMemory, 0, sizeof(CameraVectors));

	input.size = sizeof(RenderParams);
	input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	renderParamsBuffer = create_buffer(input);

	renderParamsWriteLocation = logicalDevice.mapMemory(renderParamsBuffer.bufferMemory, 0, sizeof(RenderParams));

	input.size = sizeof(ShTerms);
	input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	shTermsBuffer = create_buffer(input);

	shTermsWriteLocation = logicalDevice.mapMemory(shTermsBuffer.bufferMemory, 0, sizeof(ShTerms));

	input.size = sizeof(CameraMatrices);
	input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	cameraMatrixBuffer = create_buffer(input);

	cameraMatrixWriteLocation = logicalDevice.mapMemory(cameraMatrixBuffer.bufferMemory, 0, sizeof(CameraMatrices));

	input.size = 1024 * sizeof(glm::mat4);
	input.usage = vk::BufferUsageFlagBits::eStorageBuffer;
	modelBuffer = create_buffer(input);

	modelBufferWriteLocation = logicalDevice.mapMemory(modelBuffer.bufferMemory, 0, 1024 * sizeof(glm::mat4));

	modelTransforms.reserve(1024);
	for (int i = 0; i < 1024; ++i)
		modelTransforms.push_back(glm::mat4(1.f));

	/*
	typedef struct VkDescriptorBufferInfo {
		VkBuffer        buffer;
		VkDeviceSize    offset;
		VkDeviceSize    range;
	} VkDescriptorBufferInfo;
	*/
	cameraVectorDescriptor.buffer = cameraVectorBuffer.buffer;
	cameraVectorDescriptor.offset = 0;
	cameraVectorDescriptor.range = sizeof(CameraVectors);

	renderParamsDescriptor.buffer = renderParamsBuffer.buffer;
	renderParamsDescriptor.offset = 0;
	renderParamsDescriptor.range = sizeof(RenderParams);

	shTermsDescriptor.buffer = shTermsBuffer.buffer;
	shTermsDescriptor.offset = 0;
	shTermsDescriptor.range = sizeof(ShTerms);

	cameraMatrixDescriptor.buffer = cameraMatrixBuffer.buffer;
	cameraMatrixDescriptor.offset = 0;
	cameraMatrixDescriptor.range = sizeof(CameraMatrices);

	ssboDescriptor.buffer = modelBuffer.buffer;
	ssboDescriptor.offset = 0;
	ssboDescriptor.range = 1024 * sizeof(glm::mat4);

}

void vkutil::SwapChainFrame::makeDepthResources()
{
	depthFormat = vkimage::find_supported_format(
		physicalDevice,
		{ vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);

	vkimage::ImageInputChunk imageInfo;
	imageInfo.logicalDevice = logicalDevice;
	imageInfo.physicalDevice = physicalDevice;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	imageInfo.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	imageInfo.width = width;
	imageInfo.height = height;
	imageInfo.format = depthFormat;
	imageInfo.arrayCount = 1;
	depthBuffer = vkimage::make_image(imageInfo);
	depthBufferMemory = vkimage::make_image_memory(imageInfo, depthBuffer);
	depthBufferView = vkimage::make_image_view(
		logicalDevice, depthBuffer, depthFormat, vk::ImageAspectFlagBits::eDepth,
		vk::ImageViewType::e2D, 1
	);
}

void vkutil::SwapChainFrame::recordWriteOperations()
{
	/*
	typedef struct VkWriteDescriptorSet {
		VkStructureType                  sType;
		const void* pNext;
		VkDescriptorSet                  dstSet;
		uint32_t                         dstBinding;
		uint32_t                         dstArrayElement;
		uint32_t                         descriptorCount;
		VkDescriptorType                 descriptorType;
		const VkDescriptorImageInfo* pImageInfo;
		const VkDescriptorBufferInfo* pBufferInfo;
		const VkBufferView* pTexelBufferView;
	} VkWriteDescriptorSet;
	*/
	vk::WriteDescriptorSet cameraVectorWriteOp, cameraMatrixWriteOp, ssboWriteOp,
	 renderParamsWriteOp, shTermsWriteOp;

	cameraVectorWriteOp.dstSet = descriptorSet[pipelineType::SKY];
	cameraVectorWriteOp.dstBinding = 0;
	cameraVectorWriteOp.dstArrayElement = 0; //byte offset within binding for inline uniform blocks
	cameraVectorWriteOp.descriptorCount = 1;
	cameraVectorWriteOp.descriptorType = vk::DescriptorType::eUniformBuffer;
	cameraVectorWriteOp.pBufferInfo = &cameraVectorDescriptor;

	renderParamsWriteOp.dstSet = descriptorSet[pipelineType::SKY];
	renderParamsWriteOp.dstBinding = 1;
	renderParamsWriteOp.dstArrayElement = 0; //byte offset within binding for inline uniform blocks
	renderParamsWriteOp.descriptorCount = 1;
	renderParamsWriteOp.descriptorType = vk::DescriptorType::eUniformBuffer;
	renderParamsWriteOp.pBufferInfo = &renderParamsDescriptor;

	// When making this automatic, don't forget about increasing dstBinding
	shTermsWriteOp.dstSet = descriptorSet[pipelineType::SKY];
	shTermsWriteOp.dstBinding = 2;
	shTermsWriteOp.dstArrayElement = 0; //byte offset within binding for inline uniform blocks
	shTermsWriteOp.descriptorCount = 1;
	shTermsWriteOp.descriptorType = vk::DescriptorType::eUniformBuffer;
	shTermsWriteOp.pBufferInfo = &shTermsDescriptor;

	cameraMatrixWriteOp.dstSet = descriptorSet[pipelineType::STANDARD];
	cameraMatrixWriteOp.dstBinding = 0;
	cameraMatrixWriteOp.dstArrayElement = 0; //byte offset within binding for inline uniform blocks
	cameraMatrixWriteOp.descriptorCount = 1;
	cameraMatrixWriteOp.descriptorType = vk::DescriptorType::eUniformBuffer;
	cameraMatrixWriteOp.pBufferInfo = &cameraMatrixDescriptor;

	ssboWriteOp.dstSet = descriptorSet[pipelineType::STANDARD];
	ssboWriteOp.dstBinding = 1;
	ssboWriteOp.dstArrayElement = 0; //byte offset within binding for inline uniform blocks
	ssboWriteOp.descriptorCount = 1;
	ssboWriteOp.descriptorType = vk::DescriptorType::eStorageBuffer;
	ssboWriteOp.pBufferInfo = &ssboDescriptor;

	writeOps = { cameraVectorWriteOp, cameraMatrixWriteOp, ssboWriteOp, renderParamsWriteOp,
		shTermsWriteOp };

}

void vkutil::SwapChainFrame::writeDescriptorSet() { logicalDevice.updateDescriptorSets(writeOps, nullptr); }

void vkutil::SwapChainFrame::destroyBufferAndFreeMemory(Buffer buffer)
{
	logicalDevice.unmapMemory(buffer.bufferMemory);
	logicalDevice.freeMemory(buffer.bufferMemory);
	logicalDevice.destroyBuffer(buffer.buffer);
}

void vkutil::SwapChainFrame::destroy()
{
	logicalDevice.destroyImageView(imageView);
	logicalDevice.destroyFramebuffer(framebuffer[pipelineType::SKY]);
	logicalDevice.destroyFramebuffer(framebuffer[pipelineType::STANDARD]);
	logicalDevice.destroyFence(inFlight);
	logicalDevice.destroySemaphore(imageAvailable);
	logicalDevice.destroySemaphore(renderFinished);

	destroyBufferAndFreeMemory(cameraVectorBuffer);
	destroyBufferAndFreeMemory(renderParamsBuffer);
	destroyBufferAndFreeMemory(shTermsBuffer);
	destroyBufferAndFreeMemory(cameraMatrixBuffer);
	destroyBufferAndFreeMemory(modelBuffer);

	logicalDevice.destroyImage(depthBuffer);
	logicalDevice.freeMemory(depthBufferMemory);
	logicalDevice.destroyImageView(depthBufferView);
}