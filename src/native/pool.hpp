#pragma once 

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <atomic>

class ThreadPool {
public:
    void initialize(int count) {
        for (int i = 0; i < count; i++) {
            // add new worker
            workers.emplace_back([this]() {
                // initialize task loop
                while (true) {
                    std::function<void()> task;
                    // wait for new task
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);

                        // use condition variable to make threads go zzz until we actually have a task to do
                        cv.wait(lock, [this] { 
                            return stop || !tasks.empty();
                        });

                        // ensure we have a task
                        if (stop && tasks.empty()) {
                            break;
                        }

                        // move task from queue to var
                        task = std::move(tasks.front());

                        // if i recall correctly "move" moves the pointer data from the queue
                        // but the pointer is still there pointing to nothing
                        // so remove it anyway
                        tasks.pop();
                    }

                    // execute it :)
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        stop = true;

        // wake up all lazy ass threads
        cv.notify_all();

        // free them
        for (std::thread& t : workers) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    template<class T>
    void enqueue(T&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<T>(f));
        }
        cv.notify_one();
    }
private:
    std::atomic<bool> stop{false};
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable cv;
};

inline ThreadPool pool;
