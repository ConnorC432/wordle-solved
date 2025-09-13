//
// Created by connor on 13/09/2025.
//

#ifndef WORDLE_SOLVED_THREADPOOL_H
#define WORDLE_SOLVED_THREADPOOL_H
#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stdexcept>

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable cv;
    bool stop;

public:
    ThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        cv.wait(lock, [this]() { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        cv.notify_all();
        for (auto &thread : workers)
            if (thread.joinable())
                thread.join();
    }

    template<typename F>
    auto enqueue(F&& f) -> std::future<decltype(f())> {
        using RetType = decltype(f());
        auto task = std::make_shared<std::packaged_task<RetType()>>(std::forward<F>(f));
        std::future<RetType> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop) {
                return std::future<RetType>();
            }
            tasks.push([task]() { (*task)(); });
        }
        cv.notify_one();
        return res;
    }
};



#endif //WORDLE_SOLVED_THREADPOOL_H