#pragma once
// ThreadPool.h
#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <future>
#include <condition_variable>
#include <atomic>

class ThreadPool {
public:
    ThreadPool(size_t threadCount = std::thread::hardware_concurrency())
        : stopFlag(false)
    {
        for (size_t i = 0; i < threadCount; ++i) {
            workers.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lk(this->mtx);
                        this->cv.wait(lk, [this] {
                            return this->stopFlag.load() || !this->tasks.empty();
                            });
                        if (this->stopFlag.load() && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
                });
        }
    }

    ~ThreadPool() {
        stopFlag.store(true);
        cv.notify_all();
        for (auto& w : workers) if (w.joinable()) w.join();
    }

    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        using RetT = typename std::invoke_result<F, Args...>::type;
        auto taskPtr = std::make_shared<std::packaged_task<RetT()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<RetT> fut = taskPtr->get_future();
        {
            std::lock_guard<std::mutex> lk(mtx);
            tasks.emplace([taskPtr]() { (*taskPtr)(); });
        }
        cv.notify_one();
        return fut;
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> stopFlag;
};
