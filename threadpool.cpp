// threadpool.cpp
// Compile with:
// g++ -std=c++11 -pthread threadpool.cpp -o threadpool

#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <deque>

class ThreadPool; // forward declare

class Worker {
public:
    Worker(ThreadPool &s): pool(s) { }
    void operator()();
private:
    ThreadPool &pool;
};

class ThreadPool {
public:
    ThreadPool(size_t threads);
    template<class F> void enqueue(F f);
    ~ThreadPool();
private:
    friend class Worker;

    std::vector<std::thread> workers;
    std::deque<std::function<void()>> tasks;

    std::condition_variable cond;

    std::mutex queue_mutex;
    bool stop;
};

void Worker::operator()()
{
    std::function<void()> task;
    while (true)
    {
        std::unique_lock<std::mutex> locker(pool.queue_mutex);
        pool.cond.wait(locker, [&]() { return !pool.tasks.empty() || pool.stop;});
        if (pool.stop) return;
        task = pool.tasks.front();
        pool.tasks.pop_front();
        locker.unlock();
        task();
    }
}

ThreadPool::ThreadPool(size_t threads): stop(false)
{
    for (size_t i = 0; i < threads; ++i)
        workers.push_back(std::thread(Worker(*this)));
}

ThreadPool::~ThreadPool()
{
    stop = true; // stop all threads
    cond.notify_all();
    for (auto &thread: workers)
        thread.join();
}

template<class F>
void ThreadPool::enqueue(F f)
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    tasks.push_back(std::function<void()>(f));
    cond.notify_one();
}

int main()
{
    ThreadPool pool(4);

    // queue a bunch of "work items"
    for (int i = 0; i < 8; ++i)
        pool.enqueue([i]() { std::cout << "Hello from work item " << i << std::endl; });

    // wait for keypress to give worker threads the opportunity to finish tasks
    std::cin.ignore();

    return 0;
}

