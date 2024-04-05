#ifndef UTIL_THREAD_POOL_H_
#define UTIL_THREAD_POOL_H_

#include <queue>
#include <atomic>
#include <thread>
#include <iostream>
#include <functional>
#include <condition_variable>

#include "../delegates.h"
#include "../debug/Logger.h"
#include "../interfaces/Task.h"

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
class ThreadPool : public Task {
    debug::Logger logger;
    std::queue<T> jobs;
    std::queue<ThreadPoolResult<R>> results;
    std::mutex resultsMutex;
    std::vector<std::thread> threads;
    std::condition_variable jobsMutexCondition;
    std::mutex jobsMutex;
    std::vector<std::unique_lock<std::mutex>> workersBlocked;
    consumer<R&> resultConsumer;
    consumer<T&> onJobFailed = nullptr;
    runnable onComplete = nullptr;
    std::atomic<int> busyWorkers = 0;
    std::atomic<uint> jobsDone = 0;
    bool working = true;
    bool standaloneResults = true;

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

                busyWorkers++;
            }
            try {
                R result = (*worker)(job);
                {
                    std::lock_guard<std::mutex> lock(resultsMutex);
                    results.push(ThreadPoolResult<R> {variable, index, locked, result});
                    if (!standaloneResults) {
                        locked = true;
                    }
                    busyWorkers--;
                }
                if (!standaloneResults){
                    std::unique_lock<std::mutex> lock(mutex);
                    variable.wait(lock, [&] {
                        return !working || !locked;
                    });
                }
            } catch (std::exception& err) {
                busyWorkers--;
                if (onJobFailed) {
                    onJobFailed(job);
                }
                logger.error() << "uncaught exception: " << err.what();
            }
            jobsDone++;
        }
    }
public:
    ThreadPool(
        std::string name,
        supplier<std::shared_ptr<Worker<T, R>>> workersSupplier, 
        consumer<R&> resultConsumer
    ) : logger(name), resultConsumer(resultConsumer) {
        const uint num_threads = std::thread::hardware_concurrency();
        for (uint i = 0; i < num_threads; i++) {
            threads.emplace_back(&ThreadPool<T,R>::threadLoop, this, i, workersSupplier());
            workersBlocked.emplace_back();
        }
    }
    ~ThreadPool(){
        terminate();
    }

    void terminate() override {
        if (!working) {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(jobsMutex);
            working = false;
        }
        {
            std::lock_guard<std::mutex> lock(resultsMutex);
            while (!results.empty()) {
                ThreadPoolResult<R> entry = results.front();
                results.pop();
                if (!standaloneResults) {
                    entry.locked = false;
                    entry.variable.notify_all();
                }
            }
        }

        jobsMutexCondition.notify_all();
        for (auto& thread : threads) {
            thread.join();
        }
    }

    void update() override {
        std::lock_guard<std::mutex> lock(resultsMutex);
        while (!results.empty()) {
            ThreadPoolResult<R> entry = results.front();
            results.pop();

            resultConsumer(entry.entry);

            if (!standaloneResults) {
                entry.locked = false;
                entry.variable.notify_all();
            }
        }

        if (onComplete && busyWorkers == 0) {
            std::lock_guard<std::mutex> lock(jobsMutex);
            if (jobs.empty()) {
                onComplete();
            }
        }
    }

    void enqueueJob(T job) {
        {
            std::lock_guard<std::mutex> lock(jobsMutex);
            jobs.push(job);
        }
        jobsMutexCondition.notify_one();
    }

    /// @brief If false: worker will be blocked until it's result performed 
    void setStandaloneResults(bool flag) {
        standaloneResults = flag;
    }

    /// @brief onJobFailed called on exception thrown in worker thread.
    /// Use engine.postRunnable when calling terminate()
    void setOnJobFailed(consumer<T&> callback) {
        this->onJobFailed = callback;
    }

    /// @brief onComplete called in ThreadPool.update() when all jobs done 
    /// if ThreadPool was not terminated
    void setOnComplete(runnable callback) {
        this->onComplete = callback;
    }

    uint getWorkRemaining() const override {
        return jobs.size();
    }

    uint getWorkDone() const override {
        return jobsDone;
    }
};

} // namespace util

#endif // UTIL_THREAD_POOL_H_
