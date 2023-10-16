#include "engine.h"
#include "vkInit/instance.h"
#include "vkInit/device.h"
#include "vkInit/swapchain.h"
#include "vkInit/pipeline.h"
#include "vkInit/framebuffer.h"
#include "vkInit/commands.h"
#include "vkInit/sync.h"
#include "vkInit/descriptors.h"
#include "vkMesh/mesh.h"
#include "vkMesh/obj_mesh.h"

Engine::Engine(int width, int height, GLFWwindow* window) {

	this->width = width;
	this->height = height;
	this->window = window;

	vkLogging::Logger::getLogger()->print("Making a graphics engine...");

	makeInstance();

	makeDevice();

	makeDescriptorSetLayouts();
	makePipelines();

	finalizeSetup();

	makeWorkerThreads();
	makeAssets();
	endWorkerThreads();
}

void Engine::makeInstance() {

	instance = vkInit::make_instance("ID Tech 12");
	dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);

#ifndef NDEBUG
	debugMessenger = vkLogging::make_debug_messenger(instance, dldi);
#endif

	VkSurfaceKHR c_style_surface;
	if (glfwCreateWindowSurface(instance, window, nullptr, &c_style_surface) != VK_SUCCESS) {
		vkLogging::Logger::getLogger()->print("Failed to abstract glfw surface for Vulkan.");
	}
	else {
		vkLogging::Logger::getLogger()->print(
			"Successfully abstracted glfw surface for Vulkan.");
	}
	//copy constructor converts to hpp convention
	surface = c_style_surface;
}

void Engine::makeDevice() {

	physicalDevice = vkInit::choose_physical_device(instance);
	device = vkInit::create_logical_device(physicalDevice, surface);
	std::array<vk::Queue,2> queues = vkInit::get_queues(physicalDevice, device, surface);
	graphicsQueue = queues[0];
	presentQueue = queues[1];
	makeSwapchain();
	frameNumber = 0;
}

/**
* Make a swapchain
*/
void Engine::makeSwapchain() {

	vkInit::SwapChainBundle bundle = vkInit::create_swapchain(
		device, physicalDevice, surface, width, height
	);
	swapchain = bundle.swapchain;
	swapchainFrames = bundle.frames;
	swapchainFormat = bundle.format;
	swapchainExtent = bundle.extent;
	maxFramesInFlight = static_cast<int>(swapchainFrames.size());

	for (vkUtil::SwapChainFrame& frame : swapchainFrames) {
		frame.logicalDevice = device;
		frame.physicalDevice = physicalDevice;
		frame.width = swapchainExtent.width;
		frame.height = swapchainExtent.height;

		frame.makeDepthResources();
	}

}

/**
* The swapchain must be recreated upon resize or minimization, among other cases
*/
void Engine::recreateSwapchain() {

	width = 0;
	height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	device.waitIdle();

	cleanupSwapchain();
	makeSwapchain();
	make_framebuffers();
	makeFrameResources();
	vkInit::commandBufferInputChunk commandBufferInput = { device, commandPool, swapchainFrames };
	vkInit::make_frame_command_buffers(commandBufferInput);

}

void Engine::makeDescriptorSetLayouts() {

	// DON't FORGET TO SET BINDINGS HERE!!!

	//Binding once per frame
	vkInit::descriptorSetLayoutData bindings;

	// Sky pipeline bindings
	bindings.count = 1;

	bindings.indices.push_back(0);
	bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

	frameSetLayout[pipelineType::SKY] = vkInit::makeDescriptorSetLayout(device, bindings);

	// Standard pipeline bindings
	bindings.count = 2;

	bindings.indices.push_back(1);
	bindings.types.push_back(vk::DescriptorType::eStorageBuffer);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);

	frameSetLayout[pipelineType::STANDARD] = vkInit::makeDescriptorSetLayout(device, bindings);

	//Binding for individual draw calls
	bindings.count = 1;

	bindings.indices[0] = 0;
	bindings.types[0] = vk::DescriptorType::eCombinedImageSampler;
	bindings.counts[0] = 1;
	bindings.stages[0] = vk::ShaderStageFlagBits::eFragment;

	meshSetLayout[pipelineType::SKY] = vkInit::makeDescriptorSetLayout(device, bindings);
	meshSetLayout[pipelineType::STANDARD] = vkInit::makeDescriptorSetLayout(device, bindings);
}

void Engine::makePipelines() {

	vkInit::PipelineBuilder pipelineBuilder(device);

	// Sky
	pipelineBuilder.setOverwriteMode(false);
	pipelineBuilder.specifyVertexShader("resources/shaders/sky_shader.vert.spv");
	pipelineBuilder.specifyFragmentShader("resources/shaders/sky_shader_refraction.frag.spv"); // for now
	pipelineBuilder.specifySwapchainExtent(swapchainExtent);
	pipelineBuilder.clearDepthAttachment();
	pipelineBuilder.addDescriptorSetLayout(frameSetLayout[pipelineType::SKY]);
	pipelineBuilder.addDescriptorSetLayout(meshSetLayout[pipelineType::SKY]);
	pipelineBuilder.addColorAttachment(swapchainFormat, 0);

	vkInit::GraphicsPipelineOutBundle output = pipelineBuilder.build();

	pipelineLayout[pipelineType::SKY] = output.layout;
	renderpass[pipelineType::SKY] = output.renderpass;
	pipeline[pipelineType::SKY] = output.pipeline;
	pipelineBuilder.reset();

	// Standard
	pipelineBuilder.setOverwriteMode(true);
	pipelineBuilder.specifyVertexFormat(
		vkMesh::get_pos_color_binding_description(), 
		vkMesh::get_pos_color_attribute_descriptions()
	);
	pipelineBuilder.specifyVertexShader("resources/shaders/shader.vert.spv");
	pipelineBuilder.specifyFragmentShader("resources/shaders/shader.frag.spv");
	pipelineBuilder.specifySwapchainExtent(swapchainExtent);
	pipelineBuilder.specifyDepthAttachment(swapchainFrames[0].depthFormat, 1);
	pipelineBuilder.addDescriptorSetLayout(frameSetLayout[pipelineType::STANDARD]);
	pipelineBuilder.addDescriptorSetLayout(meshSetLayout[pipelineType::STANDARD]);
	pipelineBuilder.addColorAttachment(swapchainFormat, 0);

	output = pipelineBuilder.build();

	pipelineLayout[pipelineType::STANDARD] = output.layout;
	renderpass[pipelineType::STANDARD] = output.renderpass;
	pipeline[pipelineType::STANDARD] = output.pipeline;

}

/**
* Make a framebuffer for each frame
*/
void Engine::make_framebuffers() {

	vkInit::framebufferInput frameBufferInput;
	frameBufferInput.device = device;
	frameBufferInput.renderpass = renderpass;
	frameBufferInput.swapchainExtent = swapchainExtent;
	vkInit::make_framebuffers(frameBufferInput, swapchainFrames);
}

void Engine::finalizeSetup() {

	make_framebuffers();

	commandPool = vkInit::make_command_pool(device, physicalDevice, surface);

	vkInit::commandBufferInputChunk commandBufferInput = { device, commandPool, swapchainFrames };
	mainCommandBuffer = vkInit::make_command_buffer(commandBufferInput);
	vkInit::make_frame_command_buffers(commandBufferInput);

	makeFrameResources();

}

void Engine::makeFrameResources() {

	vkInit::descriptorSetLayoutData bindings;
	bindings.count = 2;
	bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.types.push_back(vk::DescriptorType::eStorageBuffer);
	uint32_t descriptor_sets_per_frame = 2;

	frameDescriptorPool = vkInit::make_descriptor_pool(device, static_cast<uint32_t>(swapchainFrames.size() * descriptor_sets_per_frame), bindings);

	for (vkUtil::SwapChainFrame& frame : swapchainFrames) {

		frame.imageAvailable = vkInit::make_semaphore(device);
		frame.renderFinished = vkInit::make_semaphore(device);
		frame.inFlight = vkInit::make_fence(device);

		frame.makeDescriptorResources();

		frame.descriptorSet[pipelineType::SKY] = vkInit::allocate_descriptor_set(device, frameDescriptorPool, frameSetLayout[pipelineType::SKY]);
		frame.descriptorSet[pipelineType::STANDARD] = vkInit::allocate_descriptor_set(device, frameDescriptorPool, frameSetLayout[pipelineType::STANDARD]);

		frame.recordWriteOperations();
	}

}

void Engine::makeWorkerThreads() {

	done = false;
	size_t threadCount = std::thread::hardware_concurrency() - 1;

	workers.reserve(threadCount);
	vkInit::commandBufferInputChunk commandBufferInput = { device, commandPool, swapchainFrames };
	for (size_t i = 0; i < threadCount; ++i) {
		vk::CommandBuffer commandBuffer = vkInit::make_command_buffer(commandBufferInput);
		workers.push_back(
			std::thread(
				vkJob::WorkerThread(workQueue, done, commandBuffer, graphicsQueue)
			)
		);
	}
}

void Engine::makeAssets() {

	//Meshes
	meshes = new VertexMenagerie();
	std::unordered_map<meshTypes, std::vector<const char*>> model_filenames = {
		{meshTypes::GROUND, {"resources/models/ground.obj","resources/models/ground.mtl"}},
		{meshTypes::GIRL, {"resources/models/girl.obj","resources/models/girl.mtl"}},
		{meshTypes::SKULL, {"resources/models/skull.obj","resources/models/skull.mtl"}},
		{meshTypes::VIKING_ROOM, {"resources/models/viking_room.obj","resources/models/viking_room.mtl"}}
	};
	std::unordered_map<meshTypes, glm::mat4> preTransforms = {
		{meshTypes::GROUND, glm::mat4(1.f)},
		{meshTypes::GIRL, glm::rotate(
			glm::mat4(1.f), 
			glm::radians(180.f), 
			glm::vec3(0.f, 0.f, 1.f)
		)},
		{meshTypes::SKULL, glm::mat4(1.f)},
		{meshTypes::VIKING_ROOM, glm::rotate(
			glm::mat4(1.f), 
			glm::radians(135.f), 
			glm::vec3(0.f, 0.f, 1.f)
		)}
	};
	std::unordered_map<meshTypes, vkMesh::ObjMesh> loaded_models;

	//Materials

	std::unordered_map<meshTypes, std::vector<const char*>> filenames = {
		{meshTypes::GROUND, {"resources/textures/ground.jpg"}},
		{meshTypes::GIRL, {"resources/textures/none.png"}},
		{meshTypes::SKULL, {"resources/textures/skull.png"}},
		{meshTypes::VIKING_ROOM, {"resources/textures/viking_room.png"}},
	};

	//Make a descriptor pool to allocate sets.
	vkInit::descriptorSetLayoutData bindings;
	bindings.count = 1;
	bindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);

	meshDescriptorPool = vkInit::make_descriptor_pool(device, static_cast<uint32_t>(filenames.size()) + 1, bindings);

	//Submit loading work
	workQueue.lock.lock();
	std::vector<meshTypes> mesh_types = { {meshTypes::GROUND, meshTypes::GIRL, meshTypes::SKULL, meshTypes::VIKING_ROOM} };
	for (meshTypes type : mesh_types) {
		vkImage::TextureInputChunk textureInfo;
		textureInfo.logicalDevice = device;
		textureInfo.physicalDevice = physicalDevice;
		textureInfo.layout = meshSetLayout[pipelineType::STANDARD];
		textureInfo.descriptorPool = meshDescriptorPool;
		textureInfo.filenames = filenames[type];
		materials[type] = new vkImage::Texture();
		loaded_models[type] = vkMesh::ObjMesh();
		workQueue.add(
			new vkJob::MakeTexture(materials[type], textureInfo)
		);
		workQueue.add(
			new vkJob::MakeModel(loaded_models[type], 
				model_filenames[type][0], model_filenames[type][1], 
				preTransforms[type])
		);
	}
	workQueue.lock.unlock();

	//Work will be done by the background threads,
	//we just need to wait.
#ifndef NDEBUG
	std::cout << "Waiting for work to finish." << std::endl;
#endif
	while (true) {

		if (!workQueue.lock.try_lock()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			continue;
		}

		if (workQueue.done()) {
#ifndef NDEBUG
			std::cout << "Work finished" << std::endl;
#endif
			workQueue.clear();
			workQueue.lock.unlock();
			break;
		}
		workQueue.lock.unlock();
	}

	//Consume loaded meshes
	for (std::pair<meshTypes, vkMesh::ObjMesh> pair : loaded_models) {
		meshes->consume(pair.first, pair.second.vertices, pair.second.indices);
	}

	vertexBufferFinalizationChunk finalizationInfo;
	finalizationInfo.logicalDevice = device;
	finalizationInfo.physicalDevice = physicalDevice;
	finalizationInfo.commandBuffer = mainCommandBuffer;
	finalizationInfo.queue = graphicsQueue;
	meshes->finalize(finalizationInfo);

	//Proceed when work is done

	vkImage::TextureInputChunk textureInfo;
	textureInfo.commandBuffer = mainCommandBuffer;
	textureInfo.queue = graphicsQueue;
	textureInfo.logicalDevice = device;
	textureInfo.physicalDevice = physicalDevice;
	textureInfo.descriptorPool = meshDescriptorPool;
	textureInfo.layout = meshSetLayout[pipelineType::SKY];
	textureInfo.filenames = {
			"resources/textures/skybox/posx.jpg", //x+
			"resources/textures/skybox/negx.jpg", //x-
			"resources/textures/skybox/posy.jpg", //y+
			"resources/textures/skybox/negy.jpg", //y-
			"resources/textures/skybox/posz.jpg", //z+
			"resources/textures/skybox/negz.jpg", //z-
	};
	cubemap = new vkImage::CubeMap(textureInfo);
}

void Engine::endWorkerThreads() {

	done = true;
	size_t threadCount = std::thread::hardware_concurrency() - 1;

	for (size_t i = 0; i < threadCount; ++i) {
		workers[i].join();
	}

#ifndef NDEBUG
	std::cout << "Threads ended successfully." << std::endl;
#endif
}

void Engine::updateCameraData(Camera& camera)
{
	view = camera.getViewMat();
	camera.getTransformationMatrixColumns(camVecForwards, camVecRight, camVecUp, camPos);
}

void Engine::prepareFrame(uint32_t imageIndex, Scene* scene) {

	vkUtil::SwapChainFrame& _frame = swapchainFrames[imageIndex];

	_frame.cameraVectorData.forwards = camVecForwards;
	_frame.cameraVectorData.right = camVecRight;
	_frame.cameraVectorData.up = camVecUp;
	_frame.cameraVectorData.position = camPos;
	memcpy(_frame.cameraVectorWriteLocation, &(_frame.cameraVectorData), sizeof(vkUtil::CameraVectors));

	glm::mat4 projection = glm::perspective(glm::radians(45.f), static_cast<float>(swapchainExtent.width) / static_cast<float>(swapchainExtent.height), 0.1f, 100.f);
	projection[1][1] *= -1;

	_frame.cameraMatrixData.view = view;
	_frame.cameraMatrixData.projection = projection;
	_frame.cameraMatrixData.viewProjection = projection * view;
	memcpy(_frame.cameraMatrixWriteLocation, &(_frame.cameraMatrixData), sizeof(vkUtil::CameraMatrices));

	size_t i = 0;
	for (std::pair<meshTypes, std::vector<glm::vec3>> pair : scene->positions) {
		for (glm::vec3& position : pair.second) {
			_frame.modelTransforms[i++] = glm::translate(glm::mat4(1.f), position);
		}
	}
	memcpy(_frame.modelBufferWriteLocation, _frame.modelTransforms.data(), i * sizeof(glm::mat4));

	_frame.writeDescriptorSet();
}

void Engine::prepareScene(vk::CommandBuffer commandBuffer) {

	vk::Buffer vertexBuffers[] = {meshes->vertexBuffer.buffer};
	vk::DeviceSize offsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
	commandBuffer.bindIndexBuffer(meshes->indexBuffer.buffer, 0, vk::IndexType::eUint32);
}

void Engine::recordDrawCommandsSky(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) {

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderpass[pipelineType::SKY];
	renderPassInfo.framebuffer = swapchainFrames[imageIndex].framebuffer[pipelineType::SKY];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = swapchainExtent;

	vk::ClearValue colorClear;
	std::array<float, 4> colors = { 1.f, 0.5f, 0.25f, 1.f };

	std::vector<vk::ClearValue> clearValues = { {colorClear} };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline[pipelineType::SKY]);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout[pipelineType::SKY], 0, swapchainFrames[imageIndex].descriptorSet[pipelineType::SKY], nullptr);

	cubemap->use(commandBuffer, pipelineLayout[pipelineType::SKY]);
	commandBuffer.draw(6, 1, 0, 0);

	commandBuffer.endRenderPass();
}

void Engine::recordDrawCommandsScene(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) {

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderpass[pipelineType::STANDARD];
	renderPassInfo.framebuffer = swapchainFrames[imageIndex].framebuffer[pipelineType::STANDARD];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = swapchainExtent;

	vk::ClearValue colorClear;
	std::array<float, 4> colors = { 1.f, 0.5f, 0.25f, 1.f };
	colorClear.color = vk::ClearColorValue(colors);
	vk::ClearValue depthClear;

	depthClear.depthStencil = vk::ClearDepthStencilValue({ 1.f, 0 });
	std::vector<vk::ClearValue> clearValues = { {colorClear, depthClear} };

	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearValues.data();

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline[pipelineType::STANDARD]);
	
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout[pipelineType::STANDARD], 0, swapchainFrames[imageIndex].descriptorSet[pipelineType::STANDARD], nullptr);

	prepareScene(commandBuffer);

	uint32_t startInstance = 0;
	for (std::pair<meshTypes, std::vector<glm::vec3>> pair : scene->positions) {
		renderObjects(
			commandBuffer, pair.first, startInstance, static_cast<uint32_t>(pair.second.size())
		);
	}

	commandBuffer.endRenderPass();
}

void Engine::renderObjects(vk::CommandBuffer commandBuffer, meshTypes objectType, uint32_t& startInstance, uint32_t instanceCount) {

	int indexCount = meshes->indexCounts.find(objectType)->second;
	int firstIndex = meshes->firstIndices.find(objectType)->second;
	materials[objectType]->use(commandBuffer, pipelineLayout[pipelineType::STANDARD]);
	commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, 0, startInstance);
	startInstance += instanceCount;
}

void Engine::render(Scene* scene) {

	std::ignore = device.waitForFences(1, &(swapchainFrames[frameNumber].inFlight), VK_TRUE, UINT64_MAX);
	std::ignore = device.resetFences(1, &(swapchainFrames[frameNumber].inFlight));

	//acquireNextImageKHR(vk::SwapChainKHR, timeout, semaphore_to_signal, fence)
	uint32_t imageIndex;
	try {
		vk::ResultValue acquire = device.acquireNextImageKHR(
			swapchain, UINT64_MAX, 
			swapchainFrames[frameNumber].imageAvailable, nullptr
		);
		imageIndex = acquire.value;
	}
	catch (vk::OutOfDateKHRError error) {
		std::cout << "Recreate" << std::endl;
		recreateSwapchain();
		return;
	}
	catch (vk::IncompatibleDisplayKHRError error) {
		std::cout << "Recreate" << std::endl;
		recreateSwapchain();
		return;
	}
	catch (vk::SystemError error) {
		std::cout << "Failed to acquire swapchain image!" << std::endl;
	}

	vk::CommandBuffer commandBuffer = swapchainFrames[frameNumber].commandBuffer;

	commandBuffer.reset();

	prepareFrame(imageIndex, scene);

	vk::CommandBufferBeginInfo beginInfo = {};

	try {
		commandBuffer.begin(beginInfo);
	}
	catch (vk::SystemError err) {
		vkLogging::Logger::getLogger()->print("Failed to begin recording command buffer!");
	}

	recordDrawCommandsSky(commandBuffer, imageIndex, scene);
	recordDrawCommandsScene(commandBuffer, imageIndex, scene);

	try {
		commandBuffer.end();
	}
	catch (vk::SystemError err) {

		vkLogging::Logger::getLogger()->print("failed to record command buffer!");
	}

	vk::SubmitInfo submitInfo = {};

	vk::Semaphore waitSemaphores[] = { swapchainFrames[frameNumber].imageAvailable };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vk::Semaphore signalSemaphores[] = { swapchainFrames[frameNumber].renderFinished };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	try {
		graphicsQueue.submit(submitInfo, swapchainFrames[frameNumber].inFlight);
	}
	catch (vk::SystemError err) {
		vkLogging::Logger::getLogger()->print("failed to submit draw command buffer!");
	}

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	vk::SwapchainKHR swapChains[] = { swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	vk::Result present;

	try {
		present = presentQueue.presentKHR(presentInfo);
	}
	catch (vk::OutOfDateKHRError error) {
		present = vk::Result::eErrorOutOfDateKHR;
	}

	if (present == vk::Result::eErrorOutOfDateKHR || present == vk::Result::eSuboptimalKHR) {
		std::cout << "Recreate" << std::endl;
		recreateSwapchain();
		return;
	}

	frameNumber = (frameNumber + 1) % maxFramesInFlight;

}

/**
* Free the memory associated with the swapchain objects
*/
void Engine::cleanupSwapchain() {

	for (vkUtil::SwapChainFrame& frame : swapchainFrames) {
		frame.destroy();
	}
	device.destroySwapchainKHR(swapchain);

	device.destroyDescriptorPool(frameDescriptorPool);

}

Engine::~Engine() {

	device.waitIdle();

	vkLogging::Logger::getLogger()->print("The app has been closed.");

	device.destroyCommandPool(commandPool);

	for (pipelineType pipeline_type : pipelineTypes) {
		device.destroyPipeline(pipeline[pipeline_type]);
		device.destroyPipelineLayout(pipelineLayout[pipeline_type]);
		device.destroyRenderPass(renderpass[pipeline_type]);
	}

	cleanupSwapchain();
	for (pipelineType pipeline_type : pipelineTypes) {
		device.destroyDescriptorSetLayout(frameSetLayout[pipeline_type]);
		device.destroyDescriptorSetLayout(meshSetLayout[pipeline_type]);
	}
	device.destroyDescriptorPool(meshDescriptorPool);

	delete meshes;

	for (const auto& [key, texture] : materials)
		delete texture;
	delete cubemap;

	device.destroy();

	instance.destroySurfaceKHR(surface);
#ifndef NDEBUG
	instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
#endif
	/*
	* from vulkan_funcs.hpp:
	* 
	* void Instance::destroy( Optional<const VULKAN_HPP_NAMESPACE::AllocationCallbacks> allocator = nullptr,
                                            Dispatch const & d = ::vk::getDispatchLoaderStatic())
	*/
	instance.destroy();

	//terminate glfw
	glfwTerminate();
}