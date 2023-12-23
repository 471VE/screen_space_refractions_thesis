#include "pipeline.h"
#include "../../control/logging.h"

vkinit::PipelineBuilder::PipelineBuilder(vk::Device device)
{
	this->device = device;
	reset();

	// Some stages are fixed with sensible defaults and don't need to be reconfigured
	configureInputAssembly();
	makeRasterizerInfo();
	configureMultisampling();
	configureColorBlending();
	pipelineInfo.basePipelineHandle = nullptr;
}

vkinit::PipelineBuilder::~PipelineBuilder() { reset(); }

void vkinit::PipelineBuilder::reset()
{
	pipelineInfo.flags = vk::PipelineCreateFlags();

	resetVertexFormat();
	resetShaderModules();
	resetRenderpassAttachments();
	resetDescriptorSetLayouts();
}

void vkinit::PipelineBuilder::resetVertexFormat()
{
	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;
}

void vkinit::PipelineBuilder::resetRenderpassAttachments()
{
	attachmentDescriptions.clear();
	attachmentReferences.clear();
}

void vkinit::PipelineBuilder::specifyVertexFormat (
	vk::VertexInputBindingDescription bindingDescription,
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions)
{
	this->bindingDescription = bindingDescription;
	this->attributeDescriptions = attributeDescriptions;

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &(this->bindingDescription);
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(this->attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = this->attributeDescriptions.data();
	
}

void vkinit::PipelineBuilder::setOverwriteMode(bool mode) {	overwrite = mode; }

void vkinit::PipelineBuilder::resetShaderModules()
{
	if (vertexShader)
	{
		device.destroyShaderModule(vertexShader);
		vertexShader = nullptr;
	}
	if (fragmentShader)
	{
		device.destroyShaderModule(fragmentShader);
		fragmentShader = nullptr;
	}
	shaderStages.clear();
}

void vkinit::PipelineBuilder::specifyVertexShader(const char* filename)
{
	if (vertexShader)
	{
		device.destroyShaderModule(vertexShader);
		vertexShader = nullptr;
	}

	vklogging::Logger::getLogger()->print("Create vertex shader module");
	vertexShader = vkutil::create_module(filename, device);
	vertexShaderInfo = makeShaderInfo(vertexShader, vk::ShaderStageFlagBits::eVertex);
	shaderStages.push_back(vertexShaderInfo);
}

void vkinit::PipelineBuilder::specifyFragmentShader(const char* filename)
{
	if (fragmentShader)
	{	
		device.destroyShaderModule(fragmentShader);
		fragmentShader = nullptr;
	}

	vklogging::Logger::getLogger()->print("Create fragment shader module");
	fragmentShader = vkutil::create_module(filename, device);
	fragmentShaderInfo = makeShaderInfo(fragmentShader, vk::ShaderStageFlagBits::eFragment);
	shaderStages.push_back(fragmentShaderInfo);
}

vk::PipelineShaderStageCreateInfo vkinit::PipelineBuilder::makeShaderInfo(
	const vk::ShaderModule& shaderModule, const vk::ShaderStageFlagBits& stage
) {
	vk::PipelineShaderStageCreateInfo shaderInfo = {};
	shaderInfo.flags = vk::PipelineShaderStageCreateFlags();
	shaderInfo.stage = stage;
	shaderInfo.module = shaderModule;
	shaderInfo.pName = "main";
	return shaderInfo;
}

void vkinit::PipelineBuilder::specifySwapchainExtent(vk::Extent2D screen_size) { swapchainExtent = screen_size; }

void vkinit::PipelineBuilder::specifyDepthAttachment(
	const vk::Format& depthFormat, uint32_t attachment_index
) {
	depthState.flags = vk::PipelineDepthStencilStateCreateFlags();
	depthState.depthTestEnable = true;
	depthState.depthWriteEnable = true;
	depthState.depthCompareOp = vk::CompareOp::eLess;
	depthState.depthBoundsTestEnable = false;
	depthState.stencilTestEnable = false;

	pipelineInfo.pDepthStencilState = &depthState;
	attachmentDescriptions.insert(
		{ attachment_index, 
		makeRenderpassAttachment(
			depthFormat, vk::AttachmentLoadOp::eClear, 
			vk::AttachmentStoreOp::eDontCare, 
			vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal)
		}
	);
	attachmentReferences.insert(
		{ attachment_index,
		makeAttachmentReference(attachment_index, vk::ImageLayout::eDepthStencilAttachmentOptimal)
		}
	);
}

void vkinit::PipelineBuilder::addColorAttachment(
	const vk::Format& format, uint32_t attachment_index
) {
	vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eDontCare;
	if (overwrite)
		loadOp = vk::AttachmentLoadOp::eLoad;

	vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;

	vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined;
	if (overwrite)
		initialLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::ImageLayout finalLayout = vk::ImageLayout::ePresentSrcKHR;

	attachmentDescriptions.insert(
		{ attachment_index,
		makeRenderpassAttachment(format, loadOp, storeOp, initialLayout, finalLayout)
		}
	);
	attachmentReferences.insert(
		{ attachment_index,
		makeAttachmentReference(attachment_index, vk::ImageLayout::eColorAttachmentOptimal)
		}
	);
}

vk::AttachmentDescription vkinit::PipelineBuilder::makeRenderpassAttachment(
	const vk::Format& format, vk::AttachmentLoadOp loadOp, 
	vk::AttachmentStoreOp storeOp, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout
) {
	vk::AttachmentDescription attachment = {};
	attachment.flags = vk::AttachmentDescriptionFlags();
	attachment.format = format;
	attachment.samples = vk::SampleCountFlagBits::e1;
	attachment.loadOp = loadOp;
	attachment.storeOp = storeOp;
	attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachment.initialLayout = initialLayout;
	attachment.finalLayout = finalLayout;

	return attachment;
}

vk::AttachmentReference vkinit::PipelineBuilder::makeAttachmentReference(
	uint32_t attachment_index, vk::ImageLayout layout
) {
	vk::AttachmentReference attachmentRef = {};
	attachmentRef.attachment = attachment_index;
	attachmentRef.layout = layout;

	return attachmentRef;
}

void vkinit::PipelineBuilder::clearDepthAttachment() { pipelineInfo.pDepthStencilState = nullptr; }

void vkinit::PipelineBuilder::addDescriptorSetLayout(vk::DescriptorSetLayout descriptorSetLayout)
{
	descriptorSetLayouts.push_back(descriptorSetLayout);
}

void vkinit::PipelineBuilder::resetDescriptorSetLayouts() {	descriptorSetLayouts.clear(); }

vkinit::GraphicsPipelineOutBundle vkinit::PipelineBuilder::build()
{
		// Vertex Input
		pipelineInfo.pVertexInputState = &vertexInputInfo;

		// Input Assembly
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

		// Viewport and Scissor
		makeViewportState();
		pipelineInfo.pViewportState = &viewportState;

		// Rasterizer
		pipelineInfo.pRasterizationState = &rasterizer;

		// Shader Modules
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();

		// Depth-Stencil is handled by depth attachment functions.

		// Multisampling
		pipelineInfo.pMultisampleState = &multisampling;

		// Color Blend
		pipelineInfo.pColorBlendState = &colorBlending;

		// Pipeline Layout
		vklogging::Logger::getLogger()->print("Create Pipeline Layout");
		vk::PipelineLayout pipelineLayout = makePipelineLayout();
		pipelineInfo.layout = pipelineLayout;

		// Renderpass
		vklogging::Logger::getLogger()->print("Create RenderPass");
		vk::RenderPass renderpass = makeRenderpass();
		pipelineInfo.renderPass = renderpass;
		pipelineInfo.subpass = 0;

		// Make the Pipeline
		vklogging::Logger::getLogger()->print("Create Graphics Pipeline");
		vk::Pipeline graphicsPipeline;
		try
		{
			graphicsPipeline = (device.createGraphicsPipeline(nullptr, pipelineInfo)).value;
		}
		catch (vk::SystemError err)
		{
			vklogging::Logger::getLogger()->print("Failed to create Pipeline");
		}

		GraphicsPipelineOutBundle output;
		output.layout = pipelineLayout;
		output.renderpass = renderpass;
		output.pipeline = graphicsPipeline;

		return output;
	}

void vkinit::PipelineBuilder::configureInputAssembly()
{
	inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
	inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
}

vk::PipelineViewportStateCreateInfo vkinit::PipelineBuilder::makeViewportState()
{
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = (float)swapchainExtent.width;
	viewport.height = (float)swapchainExtent.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = swapchainExtent;

	viewportState.flags = vk::PipelineViewportStateCreateFlags();
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	return viewportState;
}

void vkinit::PipelineBuilder::makeRasterizerInfo()
{
	rasterizer.flags = vk::PipelineRasterizationStateCreateFlags();
	rasterizer.depthClampEnable = VK_FALSE; //discard out of bounds fragments, don't clamp them
	rasterizer.rasterizerDiscardEnable = VK_FALSE; //This flag would disable fragment output
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.lineWidth = 1.f;
	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
	rasterizer.depthBiasEnable = VK_FALSE; //Depth bias can be useful in shadow maps.

}

void vkinit::PipelineBuilder::configureMultisampling()
{
	multisampling.flags = vk::PipelineMultisampleStateCreateFlags();
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
}

void vkinit::PipelineBuilder::configureColorBlending()
{
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = VK_FALSE;

	colorBlending.flags = vk::PipelineColorBlendStateCreateFlags();
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = vk::LogicOp::eCopy;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.f;
	colorBlending.blendConstants[1] = 0.f;
	colorBlending.blendConstants[2] = 0.f;
	colorBlending.blendConstants[3] = 0.f;

}

vk::PipelineLayout vkinit::PipelineBuilder::makePipelineLayout()
{
	// typedef struct VkPipelineLayoutCreateInfo {
	// 	VkStructureType                 sType;
	// 	const void*                     pNext;
	// 	VkPipelineLayoutCreateFlags     flags;
	// 	uint32_t                        setLayoutCount;
	// 	const VkDescriptorSetLayout*    pSetLayouts;
	// 	uint32_t                        pushConstantRangeCount;
	// 	const VkPushConstantRange*      pPushConstantRanges;
	// } VkPipelineLayoutCreateInfo;

	vk::PipelineLayoutCreateInfo layoutInfo;
	layoutInfo.flags = vk::PipelineLayoutCreateFlags();

	layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	layoutInfo.pSetLayouts = descriptorSetLayouts.data();

	layoutInfo.pushConstantRangeCount = 0;

	try
	{
		return device.createPipelineLayout(layoutInfo);
	}
	catch (vk::SystemError err)
	{
		vklogging::Logger::getLogger()->print("Failed to create pipeline layout!");
	}
	return nullptr;
}

vk::RenderPass vkinit::PipelineBuilder::makeRenderpass()
{
	flattenedAttachmentDescriptions.clear();
	flattenedAttachmentReferences.clear();
	size_t attachmentCount = attachmentDescriptions.size();
	flattenedAttachmentDescriptions.resize(attachmentCount);
	flattenedAttachmentReferences.resize(attachmentCount);

	for (int i = 0; i < attachmentCount; ++i)
	{
		flattenedAttachmentDescriptions[i] = attachmentDescriptions[i];
		flattenedAttachmentReferences[i] = attachmentReferences[i];
	}

	// Renderpasses are broken down into subpasses, there's always at least one.
	vk::SubpassDescription subpass = makeSubpass(flattenedAttachmentReferences);
	// Now create the renderpass
	vk::RenderPassCreateInfo renderpassInfo = makeRenderpassInfo(flattenedAttachmentDescriptions, subpass);
	try
	{
		return device.createRenderPass(renderpassInfo);
	}
	catch (vk::SystemError err)
	{
		vklogging::Logger::getLogger()->print("Failed to create renderpass!");
	}
	return nullptr;
}

vk::SubpassDescription vkinit::PipelineBuilder::makeSubpass(
	const std::vector<vk::AttachmentReference>& attachments
) {
	vk::SubpassDescription subpass = {};
	subpass.flags = vk::SubpassDescriptionFlags();
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &attachments[0];
	if (attachments.size() > 1)
		subpass.pDepthStencilAttachment = &attachments[1];
	else
		subpass.pDepthStencilAttachment = nullptr;

	return subpass;
}

vk::RenderPassCreateInfo vkinit::PipelineBuilder::makeRenderpassInfo(
	const std::vector<vk::AttachmentDescription>& attachments,
	const vk::SubpassDescription& subpass
) {
	vk::RenderPassCreateInfo renderpassInfo = {};
	renderpassInfo.flags = vk::RenderPassCreateFlags();
	renderpassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderpassInfo.pAttachments = attachments.data();
	renderpassInfo.subpassCount = 1;
	renderpassInfo.pSubpasses = &subpass;

	return renderpassInfo;
}