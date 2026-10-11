// -*- C++ -*-
// Copyright (c) 2012-2015 Jakob Progsch
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source
//    distribution.
//
// Modified for log4cplus, copyright (c) 2014-2015 Václav Zeman.

#ifndef THREAD_POOL_H_7ea1ee6b_4f17_4c09_b76b_3d44e102400c
#define THREAD_POOL_H_7ea1ee6b_4f17_4c09_b76b_3d44e102400c

#include <vector>
#include <list>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>
#include <functional>
#include <type_traits>
#include <stdexcept>
#include <algorithm>
#include <cassert>


namespace progschj {

class would_block
    : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};


class ThreadPool {
    // An intermediate trait avoids VS 2015's incorrect pack expansion when
    // result_of's function type is substituted directly through an alias.
    template <typename F, typename... Args>
    struct task_result :
#if defined(__cpp_lib_is_invocable) && __cpp_lib_is_invocable >= 201703L
            std::invoke_result<F&&, Args&&...>
#else
            std::result_of<F&& (Args&&...)>
#endif
    {};

public:
    template <typename F, typename... Args>
    using return_type = typename task_result<F, Args...>::type;

    // A zero size requests one worker, matching set_pool_size(0).
    // If startup fails, join any workers already started before propagating
    // the exception.
    explicit ThreadPool(std::size_t threads
        = (std::max)(2u, std::thread::hardware_concurrency()));
    template <typename F, typename... Args>
    auto enqueue_block(F&& f, Args&&... args) -> std::future<return_type<F, Args...>>;
    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<return_type<F, Args...>>;
    void wait_until_empty();
    void wait_until_nothing_in_flight();
    bool has_work_in_flight();
    void set_queue_size_limit(std::size_t limit);
    // If growth fails, the previous target size is preserved and the
    // exception propagates. Partially added workers retire asynchronously,
    // except that one may become the pool's background joiner. The joiner is
    // otherwise started on the first shrink, before changing the target; if
    // its startup fails, the previous target is preserved. Once started it
    // stays until destruction and does not count toward the target size.
    // Resizing never joins workers or waits for their thread-local cleanup.
    // Blocked cleanup can delay reclamation of other retired workers.
    void set_pool_size(std::size_t limit);
    // Drain queued tasks and join all workers, including previously retired
    // workers, and the joiner if started. Must be called outside this pool's
    // threads, with no overlapping external member calls or resources held
    // that worker cleanup needs.
    ~ThreadPool();

private:
    void start_worker(std::size_t worker_number,
        std::unique_lock<std::mutex> const &lock);
    void join_workers();

    template <typename F, typename... Args>
    auto enqueue_worker(bool, F&& f, Args&&... args) -> std::future<return_type<F, Args...>>;

    template <typename T>
    static std::future<T> make_exception_future (std::exception_ptr ex_ptr);

    struct worker_record
    {
        std::thread thread;
        bool retired = false;
        bool become_joiner = false;
    };
    // Slots may be reused as soon as a worker leaves its task loop. Records
    // remain stable and owned until the corresponding thread has been joined.
    std::vector<worker_record *> workers;
    std::list<worker_record> worker_records;
    // Only the joiner reclaims records once it has been started.
    std::thread joiner;
    std::size_t pending_retirements = 0;
    // target pool size
    std::size_t pool_size;
    // the task queue
    std::queue< std::function<void()> > tasks;
    // queue length limit
    std::size_t max_queue_size = 100000;
    // stop signal
    bool stop = false;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition_producers;
    std::condition_variable condition_consumers;
    std::condition_variable condition_joiner;

    std::mutex in_flight_mutex;
    std::condition_variable in_flight_condition;
    std::atomic<std::size_t> in_flight;

    struct handle_in_flight_decrement
    {
        ThreadPool & tp;

        handle_in_flight_decrement(ThreadPool & tp_)
            : tp(tp_)
        { }

        ~handle_in_flight_decrement()
        {
            std::size_t prev
                = std::atomic_fetch_sub_explicit(&tp.in_flight,
                    std::size_t{1},
                    std::memory_order_acq_rel);
            if (prev == 1)
            {
                std::unique_lock<std::mutex> guard(tp.in_flight_mutex);
                tp.in_flight_condition.notify_all();
            }
        }
    };
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(std::size_t threads)
    : pool_size((std::max)(threads, std::size_t{1}))
    , in_flight(0)
{
    std::unique_lock<std::mutex> lock(this->queue_mutex);
    this->workers.reserve(pool_size);
    try {
        for (std::size_t i = 0; i != pool_size; ++i)
            start_worker(i, lock);
    } catch (...) {
        // The destructor will not run if construction fails. Keep the members
        // alive through complete thread exit, including thread-local cleanup.
        stop = true;
        pool_size = 0;
        condition_consumers.notify_all();
        lock.unlock();
        for (auto &worker : worker_records)
            worker.thread.join();
        throw;
    }
}

// add new work item to the pool and block if the queue is full
template<class F, class... Args>
auto ThreadPool::enqueue_block(F&& f, Args&&... args) -> std::future<return_type<F, Args...>>
{
    return enqueue_worker (true, std::forward<F> (f), std::forward<Args> (args)...);
}

// add new work item to the pool and return future with would_block exception if it is full
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<return_type<F, Args...>>
{
    return enqueue_worker (false, std::forward<F> (f), std::forward<Args> (args)...);
}

template <typename F, typename... Args>
auto ThreadPool::enqueue_worker(bool block, F&& f, Args&&... args) -> std::future<return_type<F, Args...>>
{
    auto task = std::make_shared< std::packaged_task<return_type<F, Args...>()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

    std::future<return_type<F, Args...>> res = task->get_future();

    std::unique_lock<std::mutex> lock(queue_mutex);

    if (tasks.size () >= max_queue_size)
    {
        if (block)
        {
            // wait for the queue to empty or be stopped
            condition_producers.wait(lock,
                [this]
                {
                    return tasks.size () < max_queue_size
                        || stop;
                });
        }
        else
        {
            return ThreadPool::make_exception_future<return_type<F, Args...>> (
                std::make_exception_ptr (would_block("queue full")));
        }
    }


    // don't allow enqueueing after stopping the pool
    if (stop)
        throw std::runtime_error("enqueue on stopped ThreadPool");

    tasks.emplace([task](){ (*task)(); });
    std::atomic_fetch_add_explicit(&in_flight,
        std::size_t(1),
        std::memory_order_relaxed);
    condition_consumers.notify_one();

    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
    pool_size = 0;
    condition_consumers.notify_all();
    condition_producers.notify_all();
    condition_joiner.notify_one();
    const bool has_joiner = joiner.joinable();
    lock.unlock();
    if (has_joiner)
        joiner.join();
    else
        for (auto &worker : worker_records)
            worker.thread.join();
    assert(in_flight == 0);
}

inline void ThreadPool::wait_until_empty()
{
    std::unique_lock<std::mutex> lock(this->queue_mutex);
    this->condition_producers.wait(lock,
        [this]{ return this->tasks.empty(); });
}

inline void ThreadPool::wait_until_nothing_in_flight()
{
    std::unique_lock<std::mutex> lock(this->in_flight_mutex);
    this->in_flight_condition.wait(lock,
        [this]{ return this->in_flight == 0; });
}

inline bool ThreadPool::has_work_in_flight()
{
   std::lock_guard guard(in_flight_mutex);
   return in_flight > 0;
}

inline void ThreadPool::set_queue_size_limit(std::size_t limit)
{
    std::unique_lock<std::mutex> lock(this->queue_mutex);

    if (stop)
        return;

    std::size_t const old_limit = max_queue_size;
    max_queue_size = (std::max)(limit, std::size_t{1});
    if (old_limit < max_queue_size)
        condition_producers.notify_all();
}

inline void ThreadPool::set_pool_size(std::size_t limit)
{
    if (limit < 1)
        limit = 1;

    std::unique_lock<std::mutex> lock(this->queue_mutex);

    if (stop)
        return;

    std::size_t const old_size = pool_size;
    assert(this->workers.size() >= old_size);

    if (limit > old_size)
    {
        // Allocate before starting threads so insertion cannot fail while
        // holding a newly created, joinable thread.
        this->workers.reserve(limit);
        // create new worker threads
        // it is possible that some of these are still running because
        // they have not stopped yet after a pool size reduction, such
        // workers will just keep running
        try {
            for (std::size_t i = old_size; i != limit; ++i)
                start_worker(i, lock);
        } catch (...) {
            if (!joiner.joinable() && workers.size() > old_size) {
                // Without a joiner no previous shrink has succeeded, so these
                // are newly appended workers, still blocked on queue_mutex.
                // Promote the last one without allocating or creating another
                // thread while resources may be exhausted.
                worker_record *worker = workers.back();
                worker->become_joiner = true;
                joiner = std::move(worker->thread);
                workers.pop_back();
            }
            throw;
        }
    }
    else if (limit < old_size && !joiner.joinable())
        joiner = std::thread([this] { join_workers(); });
    // Publish the target only after all required workers have started.
    // On failure, surplus workers observe the old target (except a promoted
    // joiner) and retire normally.
    pool_size = limit;
    if (pool_size < old_size)
        // notify all worker threads to start downsizing
        this->condition_consumers.notify_all();
}

inline void ThreadPool::join_workers()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    for (;;) {
        condition_joiner.wait(lock, [this] {
            return pending_retirements != 0 || (stop && worker_records.empty());
        });
        if (stop && worker_records.empty())
            return;
        auto record = std::find_if(worker_records.begin(), worker_records.end(),
            [](const worker_record &worker) { return worker.retired; });
        assert(record != worker_records.end());
        --pending_retirements;
        // Keep ownership until joining finishes, including native TLS cleanup.
        // Workers and callers can keep using the pool while this join blocks.
        lock.unlock();
        record->thread.join();
        lock.lock();
        worker_records.erase(record);
    }
}

inline void ThreadPool::start_worker(
    std::size_t worker_number, std::unique_lock<std::mutex> const &lock)
{
    assert(lock.owns_lock() && lock.mutex() == &this->queue_mutex);
    (void)lock;
    assert(worker_number <= this->workers.size());

    // A worker still executing after downsizing can keep its existing slot.
    if (worker_number < workers.size() && workers[worker_number])
        return;

    // Allocate ownership before creating a joinable thread. Slot capacity was
    // reserved by the caller, so publishing the new worker cannot allocate.
    auto record = worker_records.emplace(worker_records.end());
    worker_record *const worker = &*record;
    auto worker_func =
        [this, worker_number, worker, record]
        {
            {
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                if (worker->become_joiner) {
                    // Its handle now belongs to joiner. Do not use the captured
                    // record, worker pointer, or slot after removing this node.
                    worker_records.erase(record);
                    lock.unlock();
                    join_workers();
                    return;
                }
            }
            for(;;)
            {
                std::function<void()> task;
                bool notify;

                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition_consumers.wait(lock,
                        [this, worker_number]{
                            return this->stop || !this->tasks.empty()
                                || pool_size < worker_number + 1; });

                    // deal with downsizing of thread pool or shutdown
                    if ((this->stop && this->tasks.empty())
                        || (!this->stop && pool_size < worker_number + 1))
                    {
                        // Retire independently of other slots. The thread
                        // record stays owned until the thread has been joined.
                        assert(this->workers[worker_number] == worker);
                        this->workers[worker_number] = nullptr;
                        worker->retired = true;
                        ++pending_retirements;
                        condition_joiner.notify_one();
                        // downsize the workers vector as much as possible
                        while (this->workers.size() > pool_size
                             && !this->workers.back())
                            this->workers.pop_back();
                        return;
                    }
                    else if (!this->tasks.empty())
                    {
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                        notify = this->tasks.size() + 1 ==  max_queue_size
                            || this->tasks.empty();
                    }
                    else
                        continue;
                }

                handle_in_flight_decrement guard(*this);

                if (notify)
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    condition_producers.notify_all();
                }

                task();
            }
        };

    try {
        worker->thread = std::thread(worker_func);
    } catch (...) {
        worker_records.erase(record);
        throw;
    }
    if (worker_number < this->workers.size())
        this->workers[worker_number] = worker;
    else
        this->workers.push_back(worker);
}

template <typename T>
inline std::future<T> ThreadPool::make_exception_future (std::exception_ptr ex_ptr)
{
    std::promise<T> p;
    p.set_exception (ex_ptr);
    return p.get_future ();
}

} // namespace progschj

#endif // THREAD_POOL_H_7ea1ee6b_4f17_4c09_b76b_3d44e102400c
