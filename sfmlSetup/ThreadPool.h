#pragma once
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>
#include <functional>
#include <random>
#include <atomic>

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads) : stop(false), indexDist(0, numThreads ? numThreads - 1 : 0) {
        if (numThreads == 0) numThreads = 1;

        queues.resize(numThreads);
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this, i] { this->workerLoop(i); });
        }
    }

    ~ThreadPool() {
        stop = true;
        condition.notify_all();
        for (auto& t : workers)
            if (t.joinable()) t.join();
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        auto wrapper = [task]() { (*task)(); };
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            size_t i = indexDist(rng);
            queues[i].emplace_back(std::move(wrapper));
        }
        condition.notify_one();
        return task->get_future();
    }

private:
    std::vector<std::thread> workers;
    std::vector<std::deque<std::function<void()>>> queues;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

    std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<size_t> indexDist;

    void workerLoop(size_t index) {
        while (!stop) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queueMutex);
                condition.wait(lock, [this, index] {
                    return stop || !queues[index].empty() || hasTaskToSteal(index);
                    });

                if (stop) break;

                if (!queues[index].empty()) {
                    task = std::move(queues[index].front());
                    queues[index].pop_front();
                }
                else {
                    size_t victim = findVictim(index);
                    if (victim != index && !queues[victim].empty()) {
                        task = std::move(queues[victim].back());
                        queues[victim].pop_back();
                    }
                }
            }

            if (task) {
                task();
            }
        }
    }

    bool hasTaskToSteal(size_t selfIndex) {
        for (size_t i = 0; i < queues.size(); ++i) {
            if (i == selfIndex) continue;
            if (!queues[i].empty()) return true;
        }
        return false;
    }

    size_t findVictim(size_t selfIndex) {
        for (size_t offset = 1; offset < queues.size(); ++offset) {
            size_t i = (selfIndex + offset) % queues.size();
            if (!queues[i].empty()) return i;
        }
        return selfIndex;
    }
};
