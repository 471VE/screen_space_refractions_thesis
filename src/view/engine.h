#pragma 
#include "camera.h"
#include "../config.h"
#include "vkUtil/frame.h"
#include "../model/scene.h"
#include "../model/vertex_menagerie.h"
#include "vkImage/texture.h"
#include "vkImage/cubemap.h"
#include "vkJob/job.h"
#include "vkJob/worker_thread.h"

class Engine {

public:

	Engine(int width, int height, GLFWwindow* window);

	~Engine();

	void render(Scene* scene);
	void updateCameraData(Camera& camera);
	void setDistanceCalculationMode(int mode);

private:

	//glfw-related variables
	int width;
	int height;
	GLFWwindow* window;

	//instance-related variables
	vk::Instance instance{ nullptr };
	vk::DebugUtilsMessengerEXT debugMessenger{ nullptr };
	vk::DispatchLoaderDynamic dldi;
	vk::SurfaceKHR surface;

	//device-related variables
	vk::PhysicalDevice physicalDevice{ nullptr };
	vk::Device device{ nullptr };
	vk::Queue graphicsQueue{ nullptr };
	vk::Queue presentQueue{ nullptr };
	vk::SwapchainKHR swapchain{ nullptr };
	std::vector<vkutil::SwapChainFrame> swapchainFrames;
	vk::Format swapchainFormat;
	vk::Extent2D swapchainExtent;

	//pipeline-related variables
	std::vector<pipelineType> pipelineTypes = { {pipelineType::SKY, pipelineType::STANDARD} };
	std::unordered_map<pipelineType,vk::PipelineLayout> pipelineLayout;
	std::unordered_map<pipelineType, vk::RenderPass> renderpass;
	std::unordered_map<pipelineType, vk::Pipeline> pipeline;

	//descriptor-related variables
	std::unordered_map<pipelineType, vk::DescriptorSetLayout> frameSetLayout;
	vk::DescriptorPool frameDescriptorPool; //Descriptors bound on a "per frame" basis
	std::unordered_map<pipelineType, vk::DescriptorSetLayout> meshSetLayout;
	vk::DescriptorPool meshDescriptorPool; //Descriptors bound on a "per mesh" basis

	//Command-related variables
	vk::CommandPool commandPool;
	vk::CommandBuffer mainCommandBuffer;

	//Synchronization objects
	int maxFramesInFlight, frameNumber;

	//asset pointers
	VertexMenagerie* meshes;
	std::unordered_map<meshTypes, vkimage::Texture*> materials;
	vkimage::CubeMap* cubemap;

	//Job System
	bool done = false;
	vkjob::WorkQueue workQueue;
	std::vector<std::thread> workers;

	//Camera-related variables
	glm::mat4 view;
	glm::vec4 camVecForwards, camVecRight, camVecUp, camPos;

	//Render-related variables
	uint32_t distanceCalculationMode = 1;

	//instance setup
	void makeInstance();

	//device setup
	void makeDevice();
	void makeSwapchain();
	void recreateSwapchain();

	//pipeline setup
	void makeDescriptorSetLayouts();
	void makePipelines();

	//final setup steps
	void finalizeSetup();
	void make_framebuffers();
	void makeFrameResources();

	//asset creation
	void makeWorkerThreads();
	void makeAssets();
	void endWorkerThreads();

	void prepareFrame(uint32_t imageIndex, Scene* scene);
	void prepareScene(vk::CommandBuffer commandBuffer);
	void recordDrawCommandsSky(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);
	void recordDrawCommandsScene(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);
	void renderObjects(
		vk::CommandBuffer commandBuffer, meshTypes objectType, uint32_t& startInstance, uint32_t instanceCount);

	//Cleanup functions
	void cleanupSwapchain();
};