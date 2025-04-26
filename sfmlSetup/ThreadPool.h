#pragma once
#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include <future>
#include <condition_variable>
#include <thread> // ��ȷ���� <thread> ͷ�ļ�

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads) : stop(false) {
        if (numThreads == 0) {
            // ����ѡ���׳��쳣������һ��Ĭ��ֵ
            // throw std::runtime_error("Number of threads cannot be zero.");
            numThreads = 1; // ���߸�һ�������Ĭ��ֵ
        }

        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task; // �洢��ִ�е�����

                    {
                        // ��ȡ���������ȴ���������
                        std::unique_lock<std::mutex> lock(this->queue_mutex);

                        // �ȴ�������ֹͣ��־Ϊtrue �� ������в�Ϊ��
                        this->condition.wait(lock,
                            [this] { return this->stop || !this->tasks.empty(); });

                        // ���ֹͣ��־Ϊtrue �� �������Ϊ�գ����߳��˳�ѭ��
                        if (this->stop && this->tasks.empty())
                            return;

                        // �Ӷ�����ȡ������
                        task = std::move(this->tasks.front());
                        this->tasks.pop();

                    } // �������ڴ˴��ͷ�

                    // ִ������
                    task();
                }
                });
        }
    }

    // ������������ȫ��ֹͣ���й����߳�
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true; // ����ֹͣ��־
        }
        condition.notify_all(); // �������еȴ����߳�

        // �ȴ������߳�������ǵĵ�ǰ�����˳�
        for (std::thread& worker : workers) {
            if (worker.joinable()) { // ����߳��Ƿ�ɻ��
                worker.join();
            }
        }
    }

    // ��ֹ�����͸�ֵ
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));

        // ����һ�� packaged_task ����װ�û���������ȡ future
        auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // ����̳߳��Ƿ���ֹͣ����
            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            // ����װ�õ�����lambda����ӵ��������
            // tasks.emplace([task](){ (*task)(); }); // ���ԭʼд��Ҳ��OK��
            tasks.push([task]() { (*task)(); }); // ����ʹ��push

        } // �������ڴ˴��ͷ�

        condition.notify_one(); // ����һ���ȴ��Ĺ����߳�
        return res;
    }

private:
    std::vector<std::thread> workers;          // �����̼߳���
    std::queue<std::function<void()>> tasks;   // �������

    std::mutex queue_mutex;                     // ������еĻ�����
    std::condition_variable condition;          // ���������������߳�ͬ��
    bool stop;                                  // ֹͣ��־
};