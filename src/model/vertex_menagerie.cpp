#include "vertex_menagerie.h"
#include "../preprocessing/preprocessing_common.h"
#include "../common/common_definitions.h"

VertexMenagerie::VertexMenagerie()
	: indexOffset(0)
{}

// Möller–Trumbore ray-triangle intersection algorithm:
static bool ray_intersects_triangle(const glm::vec3 &rayOrigin, const glm::dvec3 &rayVector, 
	const glm::vec3 &vertex0, const glm::vec3 &vertex1, const glm::vec3 &vertex2, double &distToIntersectionPoint)
{
    constexpr float EPSILON = 1.e-7f;
    glm::dvec3 edge1, edge2, rayVecXe2, s, sXe1;
    float det, invDet, u, v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    rayVecXe2 = glm::cross(rayVector, edge2);
    det = glm::dot(edge1, rayVecXe2);
    if (det > -EPSILON && det < EPSILON)
			return false; // This ray is parallel to this triangle.

    invDet = 1.f / det;
    s = rayOrigin - vertex0;
    u = invDet * glm::dot(s, rayVecXe2);
    if (u < 0.f || u > 1.f)
			return false;

    sXe1 = glm::cross(s, edge1);
    v = invDet * glm::dot(rayVector, sXe1);
    if (v < 0.f || u + v > 1.f)
			return false;

    // At this stage we can compute t to find out the distance to the intesection point:
    double t = double(invDet * glm::dot(edge2, sXe1));
    if (t > EPSILON) // ray intersection
    {
			distToIntersectionPoint = t;
			return true;
    }
    else // This means that there is a line intersection but not a ray intersection:
			return false;
}
	
void VertexMenagerie::consume(
	meshTypes type, std::vector<float>& vertexData, 
	std::vector<uint32_t>& indexData
) {
	int indexCount = static_cast<int>(indexData.size());
	int vertexCount = static_cast<int>(vertexData.size() / SINGLE_VERTEX_FLOAT_NUM);
	int lastIndex = static_cast<int>(indexLump.size());

	firstIndices.insert(std::make_pair(type, lastIndex));
	indexCounts.insert(std::make_pair(type, indexCount));

	static std::vector<glm::dvec3> hammersleySequence = construct_hemisphere_hammersley_sequence(500);
	for (int vertexNo = 0; vertexNo < vertexCount; vertexNo++)
	{
		glm::vec3 vertexPos = {vertexData[SINGLE_VERTEX_FLOAT_NUM * vertexNo],
													 vertexData[SINGLE_VERTEX_FLOAT_NUM * vertexNo + 1],
													 vertexData[SINGLE_VERTEX_FLOAT_NUM * vertexNo + 2]};
		glm::vec3 inVertexNormal = {-vertexData[SINGLE_VERTEX_FLOAT_NUM * vertexNo + 8],
																-vertexData[SINGLE_VERTEX_FLOAT_NUM * vertexNo + 9],
																-vertexData[SINGLE_VERTEX_FLOAT_NUM * vertexNo + 10]};

		// Constructing right-handed orthonormal basis
		static constexpr glm::vec3 UP = glm::vec3(0.f, 1.f, 0.f);
		glm::vec3 x_axis = (abs(glm::dot(UP, inVertexNormal)) == 1.f) ? glm::vec3(1.f, 0.f, 0.f) : glm::normalize(glm::cross(UP, inVertexNormal));
		glm::vec3 y_axis = glm::normalize(cross(inVertexNormal, x_axis));
		glm::mat3 transform = glm::mat3(x_axis, y_axis, inVertexNormal);

		std::function<DataToEncode(glm::dvec3)> getDataToEncode = [&vertexCount, &vertexData, &vertexPos, &vertexNo, &transform, &indexCount, &indexData](glm::dvec3 direction)
		{
			double width = 0;
			double maxWidth = 0;
			glm::vec3 refractedDirection = {0.f, 0.f, 0.f};
			for (int triangleIndexNo = 0; triangleIndexNo + 2 < indexCount; triangleIndexNo += 3)
			{
				glm::vec3 triangleVertex0 = {
					vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo    ]    ],  // x
					vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo    ] + 1],  // y
					vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo    ] + 2]}; // z

				glm::vec3 triangleVertex1 = {
					vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo + 1]    ],
					vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo + 1] + 1],
					vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo + 1] + 2]};

				glm::vec3 triangleVertex2 = {
					vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo + 2]    ],
					vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo + 2] + 1],
					vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo + 2] + 2]};

				// Here we go from vertex reference frame to object reference frame
				glm::vec3 globalDirection = transform * direction;
				if (ray_intersects_triangle(vertexPos, globalDirection,
						triangleVertex0, triangleVertex1, triangleVertex2, width)) [[unlikely]]
					if (width > maxWidth)
					{
						maxWidth = width;

						glm::vec3 triangleNormal0 = {
							vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo] + 8 ],  // x
							vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo] + 9 ],  // y
							vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo] + 10]}; // z

						glm::vec3 triangleNormal1 = {
							vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo + 1] + 8 ],
							vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo + 1] + 9 ],
							vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo + 1] + 10]};

						glm::vec3 triangleNormal2 = {
							vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo + 2] + 8 ],
							vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo + 2] + 9 ],
							vertexData[SINGLE_VERTEX_FLOAT_NUM * indexData[triangleIndexNo + 2] + 10]};

						glm::vec3 triangleNormalAvg = glm::normalize((triangleNormal0 + triangleNormal1 + triangleNormal2) / 3.f);

						// Normal is directed inward, eta = IOR of glass since we go from glass to air
						refractedDirection = glm::refract(globalDirection, -triangleNormalAvg, IOR);
						if (glm::dot(refractedDirection, refractedDirection) > FLT_EPSILON)
							refractedDirection = glm::normalize(refractedDirection);
					}
			}
			return DataToEncode(maxWidth, refractedDirection.x, refractedDirection.y, refractedDirection.z);
		};
		std::vector<float> sphCoeffs = calculate_sh_terms(hammersleySequence, getDataToEncode);

		static constexpr int SPHERICAL_HARMONICS_COEEFS_NUM = 9;
		for (int i = 0; i < SPHERICAL_HARMONICS_COEEFS_NUM * 4; i++)
			vertexData[SINGLE_VERTEX_FLOAT_NUM * vertexNo + 11 + i] = sphCoeffs[i];

		if (vertexNo % 100 == 0)
			std::cout << "Vertex: " << vertexNo << "/" << vertexCount << '\n';
	}

	for (float attribute : vertexData)
		vertexLump.push_back(attribute);

	for (uint32_t index : indexData)
		indexLump.push_back(index + indexOffset);

	indexOffset += vertexCount;
}

void VertexMenagerie::finalize(vertexBufferFinalizationChunk finalizationChunk)
{
	logicalDevice = finalizationChunk.logicalDevice;

	// Make a staging buffer for vertices:
	BufferInputChunk inputChunk;
	inputChunk.logicalDevice = finalizationChunk.logicalDevice;
	inputChunk.physicalDevice = finalizationChunk.physicalDevice;
	inputChunk.size = sizeof(float) * vertexLump.size();
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferSrc;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible 
		| vk::MemoryPropertyFlagBits::eHostCoherent;
	Buffer stagingBuffer = vkutil::create_buffer(inputChunk);

	// Fill it with vertex data:
	void* memoryLocation = logicalDevice.mapMemory(stagingBuffer.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, vertexLump.data(), inputChunk.size);
	logicalDevice.unmapMemory(stagingBuffer.bufferMemory);

	// Make the vertex buffer:
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferDst 
		| vk::BufferUsageFlagBits::eVertexBuffer;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	vertexBuffer = vkutil::create_buffer(inputChunk);

	// Copy to it:
	vkutil::copy_buffer(
		stagingBuffer, vertexBuffer, inputChunk.size, 
		finalizationChunk.queue, finalizationChunk.commandBuffer
	);

	// Destroy staging buffer:
	logicalDevice.destroyBuffer(stagingBuffer.buffer);
	logicalDevice.freeMemory(stagingBuffer.bufferMemory);

	// Make a staging buffer for indices:
	inputChunk.size = sizeof(uint32_t) * indexLump.size();
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferSrc;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible 
		| vk::MemoryPropertyFlagBits::eHostCoherent;
	stagingBuffer = vkutil::create_buffer(inputChunk);

	// Fill it with index data:
	memoryLocation = logicalDevice.mapMemory(stagingBuffer.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, indexLump.data(), inputChunk.size);
	logicalDevice.unmapMemory(stagingBuffer.bufferMemory);

	// Make the vertex buffer:
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferDst
		| vk::BufferUsageFlagBits::eIndexBuffer;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	indexBuffer = vkutil::create_buffer(inputChunk);

	// Copy to it:
	vkutil::copy_buffer(
		stagingBuffer, indexBuffer, inputChunk.size, 
		finalizationChunk.queue, finalizationChunk.commandBuffer
	);

	// Destroy staging buffer:
	logicalDevice.destroyBuffer(stagingBuffer.buffer);
	logicalDevice.freeMemory(stagingBuffer.bufferMemory);
	vertexLump.clear();
}

VertexMenagerie::~VertexMenagerie()
{
	// Destroy vertex buffer:
	logicalDevice.destroyBuffer(vertexBuffer.buffer);
	logicalDevice.freeMemory(vertexBuffer.bufferMemory);

	// Destroy index buffer:
	logicalDevice.destroyBuffer(indexBuffer.buffer);
	logicalDevice.freeMemory(indexBuffer.bufferMemory);

}