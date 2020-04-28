#pragma once

#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

#include "SafeQueue.h"

class ThreadPool {
private:
	bool m_shutdown;
	SafeQueue<std::function<void()>> m_queue;
	std::vector<std::thread> m_threads;

	// condition;
	std::mutex m_mutex;
	// 任务队列为空时, thread 需要休眠和唤醒.
	std::condition_variable m_condi_va;

	class ThreadWorker {
	private:
		int m_id;
		ThreadPool *m_pool;

	public:
		ThreadWorker(ThreadPool *pool, const int id)
			: m_pool(pool), m_id(id) {}

		// one loop per thread
		void operator()() {
			// std::cout << "operator()" << std::endl;
			std::function<void()> func;
			bool dequeued;
      		while (!m_pool->m_shutdown) {
        		{
          		std::unique_lock<std::mutex> lock(m_pool->m_mutex);
          		if (m_pool->m_queue.empty()) {
            		m_pool->m_condi_va.wait(lock);
          		}
          		dequeued = m_pool->m_queue.dequeue(func);
        		}
        		// will release lock !
        		// 必须有花括号!
        		if (dequeued) {
        			std::cout << "Deal job! - " << m_id << std::endl;
          			func();
        		}
      		}
		}
	};

public:
	ThreadPool(const int num_threads)
		: m_threads(std::vector<std::thread>(num_threads)), m_shutdown(false) {}

	ThreadPool(const ThreadPool &) = delete;
	ThreadPool(ThreadPool &&) = delete;

	ThreadPool & operator=(const ThreadPool &) = delete;
	ThreadPool & operator=(ThreadPool &&) = delete;

	// Inits thread pool
	void init() {
		for (int i = 0; i < (int)m_threads.size(); ++i) {
			// this: ThreadPool 指针, 
			m_threads[i] = std::thread(ThreadWorker(this, i));
		}
	}

	// Waits until threads finish their current task and shutdowns the pool
	void shutdown() {
		m_shutdown = true;
		m_condi_va.notify_all();
		// 所有线程都将退出 loop

		for (int i = 0; i < (int)m_threads.size(); ++i) {
			if (m_threads[i].joinable()) {
				m_threads[i].join();
			}
		}
	}

	// Submit a function to be executed asynchronously by the pool
	template<typename F, typename...Args>
	auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
		// Create a function with bounded parameters ready to execute
		std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
		// Encapsulate it into a shared ptr in order to be able to copy construct
		auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

		// Wrap packaged task into void function
		std::function<void()> wrapper_func = [task_ptr]() {
			(*task_ptr)();
		};


		// Enqueue generic wrapper function
		m_queue.enqueue(wrapper_func);

		// Wake up one thread if its waiting
		m_condi_va.notify_one();

		// std::cout << "notify_one!" << std::endl;

		// Return future from promise
		return task_ptr->get_future();
	}
};
