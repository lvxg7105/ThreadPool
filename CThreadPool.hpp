#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <memory>
#include <stdexcept>
#include <condition_variable>
#include <functional>
#include <iostream>

const int MAX_THREADS = 1000;

using Task = std::function<void()>;

class CThreadPool
{
public:
	CThreadPool(size_t count = 1) :m_bStop(false)
	{
		if (count <= 0 || count > MAX_THREADS)
		{
			throw::std::exception();
		}

		for (size_t i = 0; i < count; i++)
		{
			m_workThreads.emplace_back(&CThreadPool::Run, this);
		}
	}

	~CThreadPool()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_bStop = false;
		
		m_condition.notify_all();
		for (auto &thread : m_workThreads)
		{
			thread.join();
		}
	}

	bool AddTask(Task task)
	{
		m_mutex.lock();
		m_taskQueue.emplace(task);
		m_mutex.unlock();
		m_condition.notify_one();
		return true;
	}

private:
	void Run()
	{
		while (!m_bStop)
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_condition.wait(lock, [this] {return !m_taskQueue.empty(); });
			if (m_taskQueue.empty())
			{
				continue;
			}
			else
			{
				Task task = m_taskQueue.front();
				m_taskQueue.pop();
				task();
			}
		}
	}

private:
	std::vector<std::thread> m_workThreads;
	std::queue<Task> m_taskQueue;
	std::mutex m_mutex;
	std::condition_variable m_condition;
	bool m_bStop;
};
