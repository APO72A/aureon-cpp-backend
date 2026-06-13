#include "ThreadPool.h"

namespace aureon {

    ThreadPool::ThreadPool(std::size_t workerCount, std::size_t maxQueueSize)
        : maxQueueSize(maxQueueSize) {
        for (std::size_t i = 0; i < workerCount; ++i) {
            workers.emplace_back([this]() {
                workerLoop();
            });
        }
    }

    void ThreadPool::workerLoop() {
        while (true) {
            std::function<void()> job;

            { // --- critical section: only touch the queue under the lock ---
                std::unique_lock<std::mutex> lock(queueMutex);

              // Sleep until there's a job to do OR we're shutting down.
                condition.wait(lock, [this]() {
                    return stopping || !jobs.empty();
                });

                // If we're shutting down and there's no work left, exit.
                if (stopping && jobs.empty()) {
                    return;
                }

                // Take exactly one job off the queue.
                job = std::move(jobs.front());
                jobs.pop();
            } // --- lock released HERE, before running the job ---

            // Run the job WITHOUT holding the lock, so other workers
            // can grab jobs while this one works.
            job();
        }
    }

    bool ThreadPool::submit(std::function<void()> job) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);

            if (stopping) {
                return false; // shutting down - accept nothing new
            }
            if (jobs.size() >= maxQueueSize) {
                return false; // full - reject, caller handles it
            }
            jobs.push(std::move(job));
        }
        condition.notify_one();
        return true;
    }

    ThreadPool::~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stopping = true;
        }
        condition.notify_all();

        for (std::thread& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
}