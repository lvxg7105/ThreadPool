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
	//��ʼ���߳�
	ThreadPool(size_t threadNum) :m_bStop(false)
	{
		for (size_t i=0; i<threadNum; ++i)
		{
			m_threadPool.emplace_back(&ThreadPool::TaskSchedule, this);
		}
	}

	//����������
	template<typename F, typename ...Args>
	auto AddTask(F&& f, Args&& ...args) -> std::future<decltype(f(args...))>
	{
		using ReturnType = decltype(f(args...));
		if (this->m_bStop)
		{
			throw::std::runtime_error("ThreadPool has been stopped!");
		}

		// ��װ������ʵ���첽���� (packaged_task��ֹ����)
		auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
		
		//��ȡ�첽���ý��
		std::future<ReturnType> res = task->get_future();

		//�ٽ���
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			//����������������, ����Ϊfuntion<void()>
			m_tasks.emplace([task](){ (*task)();});
		}

		m_condition.notify_all();

		return res;
	}

	//����
	~ThreadPool()
	{
		//�ٽ���
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_bStop = true;
		}

		m_condition.notify_all();
		// �����̳߳������߳�, �ȴ�ִ����������е���������
		for (auto& thread : m_threadPool)
		{
			thread.join();
		}
	}

private:
	//�������
	void TaskSchedule()
	{
		while (true)
		{
			std::function<void()> task;

			//�ٽ���
			{
				std::unique_lock<std::mutex> lock(m_mutex);

				// �ȴ����̳߳ؽ�������������д�������
				m_condition.wait(lock, [this]() { return m_bStop || !m_tasks.empty(); });

				// �����ǰ�̳߳��Ѿ������ҵȴ��������Ϊ��, ��Ӧ��ֱ�ӷ���
				if (m_bStop && m_tasks.empty())
				{
					return;
				}

				//
				task = std::move(m_tasks.front());
				m_tasks.pop();
			}

			//ִ������
			task();
		}
	}

private:
	//�߳�ֹͣ��־
	bool m_bStop;
	//������
	std::mutex m_mutex;
	//��������
	std::condition_variable m_condition;
	//�̳߳�
	std::vector<std::thread> m_threadPool;
	//�������
	std::queue<std::function<void()>> m_tasks;
};