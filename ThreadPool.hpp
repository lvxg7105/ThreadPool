#pragma once
#include <vector>
#include <thread>
#include <functional>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <utility>

class ThreadPool
{
public:
	//初始化线程
	ThreadPool(size_t threadNum) :m_bStop(false)
	{
		for (size_t i=0; i<threadNum; ++i)
		{
			m_threadPool.emplace_back(&ThreadPool::TaskSchedule, this);
		}
	}

	//添加任务队列
	template<typename F, typename ...Args>
	auto AddTask(F&& f, Args&& ...args) -> std::future<decltype(f(args...))>
	{
		using ReturnType = decltype(f(args...));
		if (this->m_bStop)
		{
			throw::std::runtime_error("ThreadPool has been stopped!");
		}

		// 封装任务以实现异步调用 (packaged_task禁止拷贝)
		auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
		
		//获取异步调用结果
		std::future<ReturnType> res = task->get_future();

		//临界区
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			//向任务队列添加任务, 类型为funtion<void()>
			m_tasks.emplace([task](){ (*task)();});
		}

		m_condition.notify_all();

		return res;
	}

	//析构
	~ThreadPool()
	{
		//临界区
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_bStop = true;
		}

		m_condition.notify_all();
		// 阻塞线程池所在线程, 等待执行完任务队列的所有任务
		for (auto& thread : m_threadPool)
		{
			thread.join();
		}
	}

private:
	//任务调度
	void TaskSchedule()
	{
		while (true)
		{
			std::function<void()> task;

			//临界区
			{
				std::unique_lock<std::mutex> lock(m_mutex);

				// 等待至线程池结束或任务队列中存在任务
				m_condition.wait(lock, [this]() { return m_bStop || !m_tasks.empty(); });

				// 如果当前线程池已经结束且等待任务队列为空, 则应该直接返回
				if (m_bStop && m_tasks.empty())
				{
					return;
				}

				//
				task = std::move(m_tasks.front());
				m_tasks.pop();
			}

			//执行任务
			task();
		}
	}

private:
	//线程停止标志
	bool m_bStop;
	//互斥量
	std::mutex m_mutex;
	//条件变量
	std::condition_variable m_condition;
	//线程池
	std::vector<std::thread> m_threadPool;
	//任务队列
	std::queue<std::function<void()>> m_tasks;
};