#include "single_time_commands.h"

void vkutil::start_job(vk::CommandBuffer commandBuffer)
{
	commandBuffer.reset();

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBuffer.begin(beginInfo);
}

void vkutil::end_job(vk::CommandBuffer commandBuffer, vk::Queue submissionQueue)
{
	commandBuffer.end();

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	std::ignore = submissionQueue.submit(1, &submitInfo, nullptr);
	submissionQueue.waitIdle();
}