#include "job.h"

vkjob::MakeModel::MakeModel(vkmesh::ObjMesh& mesh, const char* objFilepath, const char* mtlFilepath, glm::mat4 preTransform)
	: mesh(mesh)
	, objFilepath(objFilepath)
	, mtlFilepath(mtlFilepath)
	, preTransform(preTransform)
{}

void vkjob::MakeModel::execute(vk::CommandBuffer commandBuffer, vk::Queue queue)
{
	mesh.load(objFilepath, mtlFilepath, preTransform);
	status = JobStatus::COMPLETE;
}

vkjob::MakeTexture::MakeTexture(vkimage::Texture* texture, vkimage::TextureInputChunk textureInfo)
	: texture(texture)
	, textureInfo(textureInfo)
{}

void vkjob::MakeTexture::execute(vk::CommandBuffer commandBuffer, vk::Queue queue)
{
	textureInfo.commandBuffer = commandBuffer;
	textureInfo.queue = queue;
	texture->load(textureInfo);
	status = JobStatus::COMPLETE;
}

void vkjob::WorkQueue::add(Job* job)
{
	if (length == 0)
	{
		first = job;
		last = job;
	}
	else {
		last->next = job;
		last = job;
	}

	length += 1;
}

vkjob::Job* vkjob::WorkQueue::getNext()
{
	Job* current = first;
	while (current)
	{
		if (current->status == JobStatus::PENDING)
			return current;
		current = current->next;
	}

	return nullptr;
}

bool vkjob::WorkQueue::done()
{
	Job* current = first;
	while (current)
	{
		if (current->status != JobStatus::COMPLETE)
			return false;
		current = current->next;
	}

	return true;
}

void vkjob::WorkQueue::clear()
{
	if (length == 0)
		return;

	Job* current = first;
	while (current)
	{
		Job* previous = current;
		current = current->next;
		delete previous;
	}

	first = nullptr;
	last = nullptr;
	length = 0;
}