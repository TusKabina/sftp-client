#include "Qt/ThreadPool.h"

void ThreadPool::start(uint32_t numThreads) {
	if (!m_poolStarted) {
		m_threads.resize(numThreads);
		for (uint32_t i = 0; i < numThreads; i++) {
			m_threads.at(i) = new WorkerThread(this);
			connect(m_threads.at(i), SIGNAL(workDone(int)), this, SLOT(eventFromThreadPoolReceived(int)));
			m_threads.at(i)->SetThreadId(i);
			m_threads.at(i)->start();
		}
		m_poolStarted = true;
		m_shouldTerminate = false;
	}
}

ThreadPool::ThreadPool() {
}

ThreadPool::~ThreadPool() {
	stop();
}

void ThreadPool::threadLoop(WorkerThread* thread) {
	while (true) {
		std::function<void()> job;
		{
			QMutexLocker locker(&m_queueMutex);
			while (m_jobs.empty() || m_shouldTerminate) {
				if (m_shouldTerminate) {
					return;
				}

				m_mutexCondition.wait(&m_queueMutex);
			}

			job = m_jobs.front();
			m_jobs.pop();
		}
		job();
		thread->workDone(thread->GetThreadId());
	}
}

void ThreadPool::queueJob(std::function<void()> job) {
	if (m_poolStarted) {
		{
			QMutexLocker locker(&m_queueMutex);
			m_jobs.push(job);
		}
		m_mutexCondition.wakeOne();
	}
}

void ThreadPool::stop() {
	if (m_poolStarted) {
		{
			QMutexLocker locker(&m_queueMutex);
			m_shouldTerminate = true;
		}
		m_mutexCondition.wakeAll();
		for (auto& activeThread : m_threads) {
			if (activeThread->isRunning()) {
				activeThread->wait();
				delete activeThread;
			}
			else {
				delete activeThread;
			}
		}
		m_threads.clear();
		m_poolStarted = false;
	}
}

void ThreadPool::eventFromThreadPoolReceived(int id) {
	emit workDone(id);
}