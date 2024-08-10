#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// 线程最大数量
#define MAX_THREAD_NUM (16)

class ThreadPool {
    typedef void (*TASK)();

public:
    ThreadPool(size_t iNum);
    ~ThreadPool();

    // void addTask(TASK task);

    // 添加一个新任务 无返回值
    template <typename F, typename... Args>
    void addTask(F&& f, Args&&... args);

    template <typename F, typename T, typename... Args>
    void addTask(F&& f, T&& t, Args&&... args);

    // 有返回值
    template <typename F, typename... Args>
    auto addTaskWithReturn(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

    template <typename F, typename T, typename... Args>
    auto addTaskWithReturn(F&& f, T&& t, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    std::vector<std::thread> m_vecTh; // 线程

    // 任务队列
    std::queue<std::function<void()>> m_queTasks;
    std::mutex m_mutex;             // 任务队列锁
    std::condition_variable m_cond; // 条件变量

    std::atomic<bool> m_bStop; // 是否停止 原子变量
};

template <typename F, typename... Args>
void ThreadPool::addTask(F&& f, Args&&... args) {
    auto task = std::bind(std::forward<F>(f), std::forward<Args...>(args)...);

    {
        std::unique_lock<std::mutex> lock(this->m_mutex);
        m_queTasks.emplace([task]() { task(); });
    }

    m_cond.notify_one();
}

template <typename F, typename T, typename... Args>
void ThreadPool::addTask(F&& f, T&& t, Args&&... args) {
    auto task = std::bind(std::forward<F>(f), std::forward<T>(t), std::forward<Args...>(args)...);

    {
        std::unique_lock<std::mutex> lock(this->m_mutex);
        m_queTasks.emplace([task]() { task(); });
    }

    m_cond.notify_one();
}

template <typename F, typename... Args>
auto ThreadPool::addTaskWithReturn(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using RETURN_TYPE = typename std::result_of<F(Args...)>::type;

    auto task =
        std::make_shared<std::packaged_task<RETURN_TYPE()>>(std::bind(std::forward<F>(f), std::forward<Args...>(args)...));
    std::future<RETURN_TYPE> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(this->m_mutex);
        m_queTasks.emplace([task]() { (*task)(); });
    }

    m_cond.notify_one();

    return res;
}

template <typename F, typename T, typename... Args>
auto ThreadPool::addTaskWithReturn(F&& f, T&& t, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using RETURN_TYPE = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<RETURN_TYPE()>>(
        std::bind(std::forward<F>(f), std::forward<T>(t), std::forward<Args...>(args)...));
    std::future<RETURN_TYPE> res = task->get_future();

    {
        std::unique_lock<std::mutex> lock(this->m_mutex);
        m_queTasks.emplace([task]() { (*task)(); });
    }

    m_cond.notify_one();

    return res;
}

ThreadPool::ThreadPool(size_t iNum) {
    m_bStop = false;

    if (iNum > MAX_THREAD_NUM) {
        iNum = MAX_THREAD_NUM;
    }

    for (int i = 0; i < iNum; i++) {
        // 新增一个线程待机
        m_vecTh.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    // 上锁
                    std::unique_lock<std::mutex> lock(this->m_mutex);
                    // 等待条件变量 只有要停止或是有任务才需要继续往后
                    this->m_cond.wait(lock, [this] { return this->m_bStop || !this->m_queTasks.empty(); });
                    // 停止 并且任务队列为空 就退出
                    if (this->m_bStop && this->m_queTasks.empty()) {
                        return;
                    }

                    task = std::move(this->m_queTasks.front());
                    this->m_queTasks.pop();
                }

                // 执行任务
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    // 设置停止标志
    m_bStop = true;

    m_cond.notify_all();

    for (auto& th : m_vecTh) {
        th.join();
    }
}

#endif
