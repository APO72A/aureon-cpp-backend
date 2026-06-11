#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace aureon {

    class ThreadPool {
    public:
        // Create the pool with a fixed number of worker threads.
        explicit ThreadPool(std::size_t workerCount);

        // Shut down cleanly: stop accepting jobs, wake all workers,
        // wait for them to finish, then join them.
        ~ThreadPool();

        // Add a job to the queue. A free worker will run it.
        void submit(std::function<void()> job);

        // No copying - a thread pool owns threads and a mutex,
        // which cannot be safely copied.
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

    private:
        std::vector<std::thread> workers; // the chefs
        std::queue<std::function<void()>> jobs; // the order rail

        std::mutex queueMutex; // the key to the queue
        std::condition_variable condition; // sleep/wake mechanism
        bool stopping = false; // shutdown flag

        // The loop each worker thread runs forever.
        void workerLoop();
    };
}