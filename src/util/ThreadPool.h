#ifndef UTIL_THREAD_POOL_H_
#define UTIL_THREAD_POOL_H_

#include <queue>
#include <thread>
#include <functional>
#include <condition_variable>

#include "../delegates.h"

namespace util {
    
template<class T>
struct ThreadPoolResult {
    std::condition_variable& variable;
    int workerIndex;
    bool& locked;
    T entry;
};

template<class T, class R>
class Worker {
public:
    Worker() {}
    virtual ~Worker() {}
    virtual R operator()(const T&) = 0;
};

template<class T, class R>
class ThreadPool {
    std::queue<T> jobs;
    std::queue<ThreadPoolResult<R>> results;
    std::vector<std::thread> threads;
    std::condition_variable jobsMutexCondition;
    std::mutex jobsMutex;
    bool working = true;
    std::vector<std::unique_lock<std::mutex>> workersBlocked;
    consumer<R&> resultConsumer;
    std::mutex resultsMutex;

    void threadLoop(int index, std::shared_ptr<Worker<T, R>> worker) {
        std::condition_variable variable;
        std::mutex mutex;
        bool locked = false;
        while (working) {
            T job;
            {
                std::unique_lock<std::mutex> lock(jobsMutex);
                jobsMutexCondition.wait(lock, [this] {
                    return !jobs.empty() || !working;
                });
                if (!working) {
                    break;
                }
                job = jobs.front();
                jobs.pop();
            }
            R result = (*worker)(job);
            {
                resultsMutex.lock();
                results.push(ThreadPoolResult<R> {variable, index, locked, result});
                locked = true;
                resultsMutex.unlock();
            }
            {
                std::unique_lock<std::mutex> lock(mutex);
                variable.wait(lock, [&] {
                    return !working || !locked;
                });
            }
        }
    }
public:
    ThreadPool(
        supplier<std::shared_ptr<Worker<T, R>>> workersSupplier, 
        consumer<R&> resultConsumer
    ) : resultConsumer(resultConsumer) {
        const uint num_threads = std::thread::hardware_concurrency();
        for (uint i = 0; i < num_threads; i++) {
            threads.emplace_back(&ThreadPool<T,R>::threadLoop, this, i, workersSupplier());
            workersBlocked.emplace_back();
        }
    }
    ~ThreadPool(){
        {
            std::lock_guard<std::mutex> lock(jobsMutex);
            working = false;
        }
        {
            std::lock_guard<std::mutex> lock(resultsMutex);
            while (!results.empty()) {
                ThreadPoolResult<R> entry = results.front();
                results.pop();
                entry.locked = false;
                entry.variable.notify_all();
            }
        }

        jobsMutexCondition.notify_all();
        for (auto& thread : threads) {
            thread.join();
        }
    }

    void update() {
        std::lock_guard<std::mutex> lock(resultsMutex);
        while (!results.empty()) {
            ThreadPoolResult<R> entry = results.front();
            results.pop();

            resultConsumer(entry.entry);

            entry.locked = false;
            entry.variable.notify_all();
        }
    }

    void enqueueJob(T job) {
        {
            std::lock_guard<std::mutex> lock(jobsMutex);
            jobs.push(job);
        }
        jobsMutexCondition.notify_one();
    }
};

} // namespace util

#endif // UTIL_THREAD_POOL_H_
