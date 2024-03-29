#pragma once

#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <optional>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <thread>
#include <mutex>

// Data structures used for creating buffers
// and allocating memory
struct BufferInputChunk {
	size_t size;
	vk::BufferUsageFlags usage;
	vk::Device logicalDevice;
	vk::PhysicalDevice physicalDevice;
	vk::MemoryPropertyFlags memoryProperties;
};

// Holds a vulkan buffer and memory allocation
struct Buffer {
	vk::Buffer buffer;
	vk::DeviceMemory bufferMemory;
};

//--------- Assets -------------//
enum class meshTypes {
	GIRL,
	GROUND,
	SKULL,
	VIKING_ROOM,
	SPHERE,
	CUBE
};

enum class pipelineType {
	SKY,
	STANDARD
};

// Encoding
#define SINGLE_VERTEX_FLOAT_NUM 47

struct DataToEncode {
	float width, x, y, z;
};
