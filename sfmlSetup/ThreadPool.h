//by chatgpt

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
#include <memory>
#include <algorithm>

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads) : stop(false), rng(std::random_device{}()) {
        if (numThreads == 0) numThreads = 1;

        queues.resize(numThreads);
        mutexes.reserve(numThreads);
        cvs.reserve(numThreads);
        for (size_t i = 0; i < numThreads; ++i) {
            mutexes.emplace_back(std::make_unique<std::mutex>());
            cvs.emplace_back(std::make_unique<std::condition_variable>());
        }

        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this, i] { this->workerLoop(i); });
        }
    }

    ~ThreadPool() {
        stop.store(true, std::memory_order_release);
        // �������еȴ����߳�
        for (auto& cv : cvs) {
            cv->notify_all();
        }
        for (auto& t : workers) {
            if (t.joinable()) t.join();
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // �ύ�����Զ����ؾ��⣺ѡ��ǰ���������ٶ��м���
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        auto wrapper = [task]() { (*task)(); };

        // ���ؾ��⣺�ҵ���̶���
        size_t idx = findShortestQueue();

        {
            std::lock_guard<std::mutex> lock(*mutexes[idx]);
            queues[idx].emplace_front(std::move(wrapper)); // ���ͷ������֤���߳����ȴ���
        }
        cvs[idx]->notify_one();

        return task->get_future();
    }

private:
    std::vector<std::thread> workers;
    std::vector<std::deque<std::function<void()>>> queues;
    std::vector<std::unique_ptr<std::mutex>> mutexes;
    std::vector<std::unique_ptr<std::condition_variable>> cvs;

    std::atomic<bool> stop;
    std::mt19937 rng;

    // �ҵ��������ٵĶ�������
    size_t findShortestQueue() {
        size_t bestIndex = 0;
        size_t bestSize = SIZE_MAX;
        for (size_t i = 0; i < queues.size(); ++i) {
            std::lock_guard<std::mutex> lock(*mutexes[i]);
            size_t sz = queues[i].size();
            if (sz < bestSize) {
                bestSize = sz;
                bestIndex = i;
                if (bestSize == 0) break; // �ҵ��ն���������
            }
        }
        return bestIndex;
    }

    void workerLoop(size_t index) {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(*mutexes[index]);
                cvs[index]->wait(lock, [this, index] {
                    return stop.load(std::memory_order_acquire) || !queues[index].empty();
                    });

                if (stop.load(std::memory_order_acquire) && queues[index].empty()) {
                    return; // �˳��߳�
                }

                if (!queues[index].empty()) {
                    task = std::move(queues[index].front()); // �Ӷ���ͷ��ȡ����
                    queues[index].pop_front();
                }
            }

            if (!task) {
                // û�б������񣬳��Դ���������β����ȡ
                for (size_t offset = 1; offset < queues.size(); ++offset) {
                    size_t victim = (index + offset) % queues.size();
                    if (mutexes[victim]->try_lock()) {
                        if (!queues[victim].empty()) {
                            task = std::move(queues[victim].back());
                            queues[victim].pop_back();
                            mutexes[victim]->unlock();
                            break;
                        }
                        mutexes[victim]->unlock();
                    }
                }
            }

            if (task) {
                task();
            }
        }
    }
};
