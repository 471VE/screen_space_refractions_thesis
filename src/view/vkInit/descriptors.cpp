#include "descriptors.h"
#include "../../control/logging.h"

	
vk::DescriptorSetLayout vkinit::makeDescriptorSetLayout(
	vk::Device device, const descriptorSetLayoutData &bindings
) {
		// typedef struct VkDescriptorSetLayoutCreateInfo {
		// 	VkStructureType                        sType;
		// 	const void*                            pNext;
		// 	VkDescriptorSetLayoutCreateFlags       flags;
		// 	uint32_t                               bindingCount;
		// 	const VkDescriptorSetLayoutBinding*    pBindings;
		// } VkDescriptorSetLayoutCreateInfo;
	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits();
	layoutInfo.bindingCount = bindings.bindings.size();
	layoutInfo.pBindings = bindings.bindings.data();

	try
	{
		return device.createDescriptorSetLayout(layoutInfo);
	}
	catch (vk::SystemError err)
	{
		vklogging::Logger::getLogger()->print("Failed to create descriptor set layout");
		return nullptr;
	}
}

vk::DescriptorPool vkinit::make_descriptor_pool(
	vk::Device device, uint32_t size, const std::vector<vk::DescriptorType> &descriptorTypes
) {
	std::vector<vk::DescriptorPoolSize> poolSizes;
	// typedef struct VkDescriptorPoolSize {
	// 	VkDescriptorType    type;
	// 	uint32_t            descriptorCount;
	// } VkDescriptorPoolSize;

	for (int i = 0; i < descriptorTypes.size(); i++)
	{
		vk::DescriptorPoolSize poolSize;
		poolSize.type = descriptorTypes[i];
		poolSize.descriptorCount = size;
		poolSizes.push_back(poolSize);
	}

	vk::DescriptorPoolCreateInfo poolInfo;
	// typedef struct VkDescriptorPoolCreateInfo {
	// 	VkStructureType                sType;
	// 	const void*                    pNext;
	// 	VkDescriptorPoolCreateFlags    flags;
	// 	uint32_t                       maxSets;
	// 	uint32_t                       poolSizeCount;
	// 	const VkDescriptorPoolSize*    pPoolSizes;
	// } VkDescriptorPoolCreateInfo;

	poolInfo.flags = vk::DescriptorPoolCreateFlags();
	poolInfo.maxSets = size;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();

	try
	{
		return device.createDescriptorPool(poolInfo);
	}
	catch (vk::SystemError err)
	{
		vklogging::Logger::getLogger()->print("Failed to make descriptor pool");
		return nullptr;
	}
}

vk::DescriptorSet vkinit::allocate_descriptor_set(
	vk::Device device, vk::DescriptorPool descriptorPool,
	vk::DescriptorSetLayout layout
) {
	vk::DescriptorSetAllocateInfo allocationInfo;
	// typedef struct VkDescriptorSetAllocateInfo {
	// 	VkStructureType                 sType;
	// 	const void*                     pNext;
	// 	VkDescriptorPool                descriptorPool;
	// 	uint32_t                        descriptorSetCount;
	// 	const VkDescriptorSetLayout*    pSetLayouts;
	// } VkDescriptorSetAllocateInfo;

	allocationInfo.descriptorPool = descriptorPool;
	allocationInfo.descriptorSetCount = 1;
	allocationInfo.pSetLayouts = &layout;

	try
	{
		return device.allocateDescriptorSets(allocationInfo)[0];
	}
	catch (vk::SystemError err)
	{
		vklogging::Logger::getLogger()->print("Failed to allocate descriptor set from pool");
		return nullptr;
	}
}