#pragma once
#include "../../config.h"
#include "../vkMesh/obj_mesh.h"
#include "../vkImage/image.h"
#include "../vkImage/texture.h"

namespace vkjob {

	enum class JobStatus {
		PENDING,
		IN_PROGRESS,
		COMPLETE
	};

	class Job {
	public:
		JobStatus status = JobStatus::PENDING;
		Job* next = nullptr;
		virtual void execute(vk::CommandBuffer commandBuffer, vk::Queue queue) = 0;
	};

	class MakeModel: public Job {
	public:
		const char* objFilepath;
		const char* mtlFilepath;
		glm::mat4 preTransform;
		vkmesh::ObjMesh& mesh;
		MakeModel(vkmesh::ObjMesh& mesh, const char* objFilepath, const char* mtlFilepath, glm::mat4 preTransform);
		virtual void execute(vk::CommandBuffer commandBuffer, vk::Queue queue) final;
	};

	class MakeTexture : public Job {
	public:
		vkimage::TextureInputChunk textureInfo;
		vkimage::Texture* texture;
		MakeTexture(vkimage::Texture* texture, vkimage::TextureInputChunk textureInfo);
		virtual void execute(vk::CommandBuffer commandBuffer, vk::Queue queue) final;
	};

	class WorkQueue {
	public:
		Job* first = nullptr, * last = nullptr;
		size_t length = 0;
		std::mutex lock;
		void add(Job* job);
		Job* getNext();
		bool done();
		void clear();
	};
}