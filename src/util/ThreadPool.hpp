#ifndef UTIL_THREAD_POOL_HPP_
#define UTIL_THREAD_POOL_HPP_

#include "../delegates.hpp"
#include "../debug/Logger.hpp"
#include "../interfaces/Task.hpp"

#include <queue>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <functional>
#include <condition_variable>
#include <utility>

namespace util {
    
template<class J, class T>
struct ThreadPoolResult {
    std::shared_ptr<J> job;
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
    virtual R operator()(const std::shared_ptr<T>&) = 0;
};

template<class T, class R>
class ThreadPool : public Task {
    debug::Logger logger;
    std::queue<std::shared_ptr<T>> jobs;
    std::queue<ThreadPoolResult<T, R>> results;
    std::mutex resultsMutex;
    std::vector<std::thread> threads;
    std::condition_variable jobsMutexCondition;
    std::mutex jobsMutex;
    std::vector<std::unique_lock<std::mutex>> workersBlocked;
    consumer<R&> resultConsumer;
    consumer<std::shared_ptr<T>&> onJobFailed = nullptr;
    runnable onComplete = nullptr;
    std::atomic<int> busyWorkers = 0;
    std::atomic<uint> jobsDone = 0;
    std::atomic<bool> working = true;
    bool failed = false;
    bool standaloneResults = true;
    bool stopOnFail = true;

    void threadLoop(int index, std::shared_ptr<Worker<T, R>> worker) {
        std::condition_variable variable;
        std::mutex mutex;
        bool locked = false;
        while (working) {
            std::shared_ptr<T> job;
            {
                std::unique_lock<std::mutex> lock(jobsMutex);
                jobsMutexCondition.wait(lock, [this] {
                    return !jobs.empty() || !working;
                });
                if (!working || failed) {
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
                    results.push(ThreadPoolResult<T, R> {job, variable, index, locked, result});
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
                if (stopOnFail) {
                    std::lock_guard<std::mutex> lock(jobsMutex);
                    failed = true;
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
    ) : logger(std::move(name)), resultConsumer(resultConsumer) {
        const uint num_threads = std::thread::hardware_concurrency();
        for (uint i = 0; i < num_threads; i++) {
            threads.emplace_back(&ThreadPool<T,R>::threadLoop, this, i, workersSupplier());
            workersBlocked.emplace_back();
        }
    }
    ~ThreadPool(){
        terminate();
    }

    bool isActive() const override {
        return working;
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
                ThreadPoolResult<T,R> entry = results.front();
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
        if (!working) {
            return;
        }
        if (failed) {
            throw std::runtime_error("some job failed");
        }

        bool complete = false;
        {
            std::lock_guard<std::mutex> lock(resultsMutex);
            while (!results.empty()) {
                ThreadPoolResult<T,R> entry = results.front();
                results.pop();

                try {
                    resultConsumer(entry.entry);
                } catch (std::exception& err) {
                    logger.error() << err.what();
                    if (onJobFailed) {
                        onJobFailed(entry.job);
                    }
                    if (stopOnFail) {
                        std::lock_guard<std::mutex> jobsLock(jobsMutex);
                        failed = true;
                        complete = false;
                    }
                    break;
                }

                if (!standaloneResults) {
                    entry.locked = false;
                    entry.variable.notify_all();
                }
            }

            if (onComplete && busyWorkers == 0) {
                std::lock_guard<std::mutex> jobsLock(jobsMutex);
                if (jobs.empty()) {
                    onComplete();
                    complete = true;
                }
            }
        }
        if (failed) {
            throw std::runtime_error("some job failed");
        }
        if (complete) {
            terminate();
        }
    }

    void enqueueJob(const std::shared_ptr<T>& job) {
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

    void setStopOnFail(bool flag) {
        stopOnFail = flag;
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

    uint getWorkTotal() const override {
        return jobs.size()+jobsDone+busyWorkers;
    }

    uint getWorkDone() const override {
        return jobsDone;
    }

    virtual void waitForEnd() override {
        using namespace std::chrono_literals;
        while (working) {
            std::this_thread::sleep_for(2ms);
            update();
        }
    }

    uint getWorkersCount() const {
        return threads.size();
    }
};

} // namespace util

#endif // UTIL_THREAD_POOL_HPP_
