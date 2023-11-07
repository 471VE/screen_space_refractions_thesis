#pragma once
#include "../../config.h"

namespace vkUtil {

	/**
		Describes the data to send to the shader for each frame.
	*/
	struct CameraMatrices {
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewProjection;
	};

	struct CameraVectors {
		glm::vec4 forwards;
		glm::vec4 right;
		glm::vec4 up;
		glm::vec4 position;
	};

	struct RenderParams
	{
		float aspectRatio = 16.f / 9.f;
		uint32_t distanceCalculationMode = 1;
	};

	/**
		Holds the data structures associated with a "Frame"
	*/
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

		RenderParams renderParamsData;
		Buffer renderParamsBuffer;
		void* renderParamsWriteLocation;

		std::vector<glm::mat4> modelTransforms;
		Buffer modelBuffer;
		void* modelBufferWriteLocation;

		//Resource Descriptors
		vk::DescriptorBufferInfo cameraVectorDescriptor, cameraMatrixDescriptor;
		vk::DescriptorBufferInfo ssboDescriptor;
		vk::DescriptorBufferInfo renderParamsDescriptor; 
		std::unordered_map<pipelineType, vk::DescriptorSet> descriptorSet;

		//Write Operations
		std::vector<vk::WriteDescriptorSet> writeOps;

		void makeDescriptorResources();

		void recordWriteOperations();

		void makeDepthResources();

		void writeDescriptorSet();

		void destroy();
	};

}