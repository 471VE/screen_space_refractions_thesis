#pragma once
#include "../../config.h"

namespace vkinit {

	// Describes the bindings of a descriptor set layout
	struct descriptorSetLayoutData {
		// Bindings describe a whole bunch of descriptor types, and collect them all into a
		// list of some kind.

		// typedef struct VkDescriptorSetLayoutBinding {
		// 	 uint32_t              binding;
		// 	 VkDescriptorType      descriptorType;
		// 	 uint32_t              descriptorCount;
		// 	 VkShaderStageFlags    stageFlags;
		// 	 const VkSampler*      pImmutableSamplers;
		// } VkDescriptorSetLayoutBinding;
		std::vector<vk::DescriptorSetLayoutBinding> bindings;

		void emplace_back(vk::DescriptorType descriptorType, vk::ShaderStageFlags stageFlags)
		{
			uint32_t binding = bindings.size();
			bindings.emplace_back(binding, descriptorType, 1, stageFlags);
		}
	};

	/**
		Make a descriptor set layout from the given descriptions

		\param device the logical device
		\param bindings	a vector of the bindings used in the shader
		\returns the created descriptor set layout
	*/
	vk::DescriptorSetLayout makeDescriptorSetLayout(
		vk::Device device, const descriptorSetLayoutData &bindings);

	/**
		Make a descriptor pool

		\param device the logical device
		\param size the number of descriptor sets to allocate from the pool
		\param descriptorTypes	used to get the descriptor types
		\returns the created descriptor pool
	*/
	vk::DescriptorPool make_descriptor_pool(
		vk::Device device, uint32_t size, const std::vector<vk::DescriptorType> &descriptorTypes);

	/**
		Allocate a descriptor set from a pool.

		\param device the logical device
		\param descriptorPool the pool to allocate from
		\param layout the descriptor set layout which the set must adhere to
		\returns the allocated descriptor set
	*/
	vk::DescriptorSet allocate_descriptor_set(
		vk::Device device, vk::DescriptorPool descriptorPool,
		vk::DescriptorSetLayout layout);
}