#ifndef SFTP_CLIENT_THREADPOOL_H
#define SFTP_CLIENT_THREADPOOL_H

#include <QWidget>
#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QtCore/QThread>
#include <thread>
#include <atomic>
#include <vector>
#include <queue>
#include <functional>
class WorkerThread;

class ThreadPool : public QWidget {
	Q_OBJECT
signals:
	void workDone(int);
public:
	ThreadPool();
	~ThreadPool();
	void start(uint32_t numThreads);
	void queueJob(std::function<void()> job);
	void stop();
	void threadLoop(WorkerThread* thread);

public slots:
	void eventFromThreadPoolReceived(int);

private:
	volatile std::atomic<bool> m_poolStarted = false;
	bool m_shouldTerminate = false;
	QMutex m_queueMutex;
	QWaitCondition m_mutexCondition;
	std::vector<WorkerThread*> m_threads;
	std::queue<std::function<void()>> m_jobs;
};

class WorkerThread : public QThread {
	Q_OBJECT
signals:
	void workDone(int);

private:
	ThreadPool* m_parent;
	int m_threadId;
	void run() { m_parent->threadLoop(this); };

public:
	WorkerThread(ThreadPool* parent) : m_parent(parent), m_threadId(-1) {};
	void SetThreadId(int id) { m_threadId = id; }
	int& GetThreadId() { return m_threadId; }

};


#endif // SFTP_CLIENT_THREADPOOL_H