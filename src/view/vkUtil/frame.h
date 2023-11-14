#pragma once
#include "../../config.h"
#include "../../common/common_definitions.h"

namespace vkutil
{
	// Holds the data structures associated with a "Frame"
	class SwapChainFrame {

	public:

		//For doing work
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;

		//Swapchain-type stuff
		vk::Image image;
		vk::ImageView imageView;
		std::unordered_map<pipelineType,vk::Framebuffer> framebuffer;
		vk::Image depthBuffer;
		vk::DeviceMemory depthBufferMemory;
		vk::ImageView depthBufferView;
		vk::Format depthFormat;
		int width, height;

		vk::CommandBuffer commandBuffer;

		//Sync objects
		vk::Semaphore imageAvailable, renderFinished;
		vk::Fence inFlight;

		//Resources
		CameraMatrices cameraMatrixData;
		Buffer cameraMatrixBuffer;
		void* cameraMatrixWriteLocation;

		CameraVectors cameraVectorData;
		Buffer cameraVectorBuffer;
		void* cameraVectorWriteLocation;

		RenderParams renderParamsData = {
			.aspectRatio = 16.f / 9.f,
			.distanceCalculationMode = 1
		};
		Buffer renderParamsBuffer;
		void* renderParamsWriteLocation;

		ShTerms shTermsData;
		Buffer shTermsBuffer;
		void* shTermsWriteLocation;

		std::vector<glm::mat4> modelTransforms;
		Buffer modelBuffer;
		void* modelBufferWriteLocation;

		//Resource Descriptors
		vk::DescriptorBufferInfo cameraVectorDescriptor, cameraMatrixDescriptor;
		vk::DescriptorBufferInfo ssboDescriptor;
		vk::DescriptorBufferInfo renderParamsDescriptor;
		vk::DescriptorBufferInfo shTermsDescriptor;
		std::unordered_map<pipelineType, vk::DescriptorSet> descriptorSet;

		//Write Operations
		std::vector<vk::WriteDescriptorSet> writeOps;

		void makeDescriptorResources();

		void recordWriteOperations();

		void makeDepthResources();

		void writeDescriptorSet();

		void destroyBufferAndFreeMemory(Buffer buffer);

		void destroy();
	};

}