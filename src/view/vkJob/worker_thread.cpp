#include "worker_thread.h"

vkjob::WorkerThread::WorkerThread(WorkQueue& workQueue, bool& done, vk::CommandBuffer commandBuffer, vk::Queue queue):
workQueue(workQueue), done(done){
	this->commandBuffer = commandBuffer;
	this->queue = queue;
}

void vkjob::WorkerThread::operator()()
{
	workQueue.lock.lock();
#ifndef NDEBUG
	std::cout << "----    Thread is ready to go.    ----" << std::endl;
#endif
	workQueue.lock.unlock();

	while (!done)
	{
		if (!workQueue.lock.try_lock())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			continue;
		}
		
		if (workQueue.done())
		{
			workQueue.lock.unlock();
			continue;
		}

		vkjob::Job* pendingJob = workQueue.getNext();

		if (!pendingJob)
		{
			workQueue.lock.unlock();
			continue;
		}
#ifndef NDEBUG
		std::cout << "----    Working on a job.    ----" << std::endl;
#endif
		pendingJob->status = JobStatus::IN_PROGRESS;
		workQueue.lock.unlock();
		pendingJob->execute(commandBuffer, queue);
	}
	
#ifndef NDEBUG
	std::cout << "----    Thread done.    ----" << std::endl;
#endif
}