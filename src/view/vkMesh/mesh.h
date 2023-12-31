#pragma once
#include "../../config.h"

namespace vkmesh {

	// \returns the input binding description for a (vec2 pos, vec3 color, vec2 texcoords) vertex format.
	vk::VertexInputBindingDescription get_pos_color_binding_description()
	{
		// Provided by VK_VERSION_1_0:
		// typedef struct VkVertexInputBindingDescription {
		// 	uint32_t             binding;
		// 	uint32_t             stride;
		// 	VkVertexInputRate    inputRate;
		// } VkVertexInputBindingDescription;

		vk::VertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = 20 * sizeof(float);
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;
		
		return bindingDescription;
	}

	// \returns the input attribute descriptions for a (vec2 pos, vec3 color, vec2 texcoords) vertex format.
	std::vector<vk::VertexInputAttributeDescription> get_pos_color_attribute_descriptions()
	{
		// Provided by VK_VERSION_1_0:
		// typedef struct VkVertexInputAttributeDescription {
		// 	uint32_t    location;
		// 	uint32_t    binding;
		// 	VkFormat    format;
		// 	uint32_t    offset;
		// } VkVertexInputAttributeDescription;

		std::vector<vk::VertexInputAttributeDescription> attributes;
		vk::VertexInputAttributeDescription dummy;
		attributes.push_back(dummy);
		attributes.push_back(dummy);
		attributes.push_back(dummy);
		attributes.push_back(dummy);
		attributes.push_back(dummy);
		attributes.push_back(dummy);
		attributes.push_back(dummy);

		// Pos
		attributes[0].binding = 0;
		attributes[0].location = 0;
		attributes[0].format = vk::Format::eR32G32B32Sfloat;
		attributes[0].offset = 0;

		// Color
		attributes[1].binding = 0;
		attributes[1].location = 1;
		attributes[1].format = vk::Format::eR32G32B32Sfloat;
		attributes[1].offset = 3 * sizeof(float);

		// TexCoord
		attributes[2].binding = 0;
		attributes[2].location = 2;
		attributes[2].format = vk::Format::eR32G32Sfloat;
		attributes[2].offset = 6 * sizeof(float);

		// Normal
		attributes[3].binding = 0;
		attributes[3].location = 3;
		attributes[3].format = vk::Format::eR32G32B32Sfloat;
		attributes[3].offset = 8 * sizeof(float);

		// Spherical harmonics expansion coefficients
		attributes[4].binding = 0;
		attributes[4].location = 4;
		attributes[4].format = vk::Format::eR32G32B32Sfloat;
		attributes[4].offset = 11 * sizeof(float);

		attributes[5].binding = 0;
		attributes[5].location = 5;
		attributes[5].format = vk::Format::eR32G32B32Sfloat;
		attributes[5].offset = 14 * sizeof(float);

		attributes[6].binding = 0;
		attributes[6].location = 6;
		attributes[6].format = vk::Format::eR32G32B32Sfloat;
		attributes[6].offset = 17 * sizeof(float);

		return attributes;
	}
}