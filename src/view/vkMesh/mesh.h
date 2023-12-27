#pragma once
#include "../../config.h"
#include "../../common/common_definitions.h"

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
		bindingDescription.stride = SINGLE_VERTEX_FLOAT_NUM * sizeof(float);
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
		for (int i = 0; i < 16; i++)
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

		// Width
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

		// X-coordinate of a refracted vector
		attributes[7].binding = 0;
		attributes[7].location = 7;
		attributes[7].format = vk::Format::eR32G32B32Sfloat;
		attributes[7].offset = 20 * sizeof(float);

		attributes[8].binding = 0;
		attributes[8].location = 8;
		attributes[8].format = vk::Format::eR32G32B32Sfloat;
		attributes[8].offset = 23 * sizeof(float);

		attributes[9].binding = 0;
		attributes[9].location = 9;
		attributes[9].format = vk::Format::eR32G32B32Sfloat;
		attributes[9].offset = 26 * sizeof(float);

		// Y-coordinate of a refracted vector
		attributes[10].binding = 0;
		attributes[10].location = 10;
		attributes[10].format = vk::Format::eR32G32B32Sfloat;
		attributes[10].offset = 29 * sizeof(float);

		attributes[11].binding = 0;
		attributes[11].location = 11;
		attributes[11].format = vk::Format::eR32G32B32Sfloat;
		attributes[11].offset = 32 * sizeof(float);

		attributes[12].binding = 0;
		attributes[12].location = 12;
		attributes[12].format = vk::Format::eR32G32B32Sfloat;
		attributes[12].offset = 35 * sizeof(float);

		// Z-coordinate of a refracted vector
		attributes[13].binding = 0;
		attributes[13].location = 13;
		attributes[13].format = vk::Format::eR32G32B32Sfloat;
		attributes[13].offset = 38 * sizeof(float);

		attributes[14].binding = 0;
		attributes[14].location = 14;
		attributes[14].format = vk::Format::eR32G32B32Sfloat;
		attributes[14].offset = 41 * sizeof(float);

		attributes[15].binding = 0;
		attributes[15].location = 15;
		attributes[15].format = vk::Format::eR32G32B32Sfloat;
		attributes[15].offset = 44 * sizeof(float);

		return attributes;
	}
}