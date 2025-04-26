#pragma once
#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include <future>
#include <condition_variable>
#include <thread> // 明确包含 <thread> 头文件

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads) : stop(false) {
        if (numThreads == 0) {
            // 可以选择抛出异常或设置一个默认值
            // throw std::runtime_error("Number of threads cannot be zero.");
            numThreads = 1; // 或者给一个合理的默认值
        }

        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task; // 存储待执行的任务

                    {
                        // 获取互斥锁并等待条件变量
                        std::unique_lock<std::mutex> lock(this->queue_mutex);

                        // 等待条件：停止标志为true 或 任务队列不为空
                        this->condition.wait(lock,
                            [this] { return this->stop || !this->tasks.empty(); });

                        // 如果停止标志为true 且 任务队列为空，则线程退出循环
                        if (this->stop && this->tasks.empty())
                            return;

                        // 从队列中取出任务
                        task = std::move(this->tasks.front());
                        this->tasks.pop();

                    } // 互斥锁在此处释放

                    // 执行任务
                    task();
                }
                });
        }
    }

    // 析构函数：安全地停止所有工作线程
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true; // 设置停止标志
        }
        condition.notify_all(); // 唤醒所有等待的线程

        // 等待所有线程完成它们的当前任务并退出
        for (std::thread& worker : workers) {
            if (worker.joinable()) { // 检查线程是否可汇合
                worker.join();
            }
        }
    }

    // 禁止拷贝和赋值
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));

        // 创建一个 packaged_task 来包装用户函数并获取 future
        auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // 检查线程池是否已停止运行
            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            // 将包装好的任务（lambda）添加到任务队列
            // tasks.emplace([task](){ (*task)(); }); // 你的原始写法也是OK的
            tasks.push([task]() { (*task)(); }); // 或者使用push

        } // 互斥锁在此处释放

        condition.notify_one(); // 唤醒一个等待的工作线程
        return res;
    }

private:
    std::vector<std::thread> workers;          // 工作线程集合
    std::queue<std::function<void()>> tasks;   // 任务队列

    std::mutex queue_mutex;                     // 任务队列的互斥锁
    std::condition_variable condition;          // 条件变量，用于线程同步
    bool stop;                                  // 停止标志
};