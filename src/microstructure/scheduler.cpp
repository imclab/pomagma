#include "scheduler.hpp"
#include <pomagma/platform/threading.hpp>
#include <vector>
#include <thread>
#include <chrono>
#include <tbb/concurrent_queue.h>


//#define POMAGMA_DEBUG1(message)
#define POMAGMA_DEBUG1(message) POMAGMA_DEBUG(message)


namespace pomagma
{

namespace Scheduler
{

static size_t g_worker_count = DEFAULT_THREAD_COUNT;

static std::atomic<bool> g_working_flag(false);
static std::atomic<uint_fast64_t> g_working_count(0);
static std::mutex g_working_mutex;
static std::condition_variable g_working_condition;

static SharedMutex g_strict_mutex;

class TaskStats
{
    std::atomic<uint_fast64_t> m_schedule_count;
    std::atomic<uint_fast64_t> m_execute_count;
public:
    TaskStats ()
        : m_schedule_count(0),
          m_execute_count(0)
    {
    }

    void schedule () { m_schedule_count.fetch_add(1, relaxed); }
    void execute () { m_execute_count.fetch_add(1, relaxed); }
    void clear ()
    {
        m_schedule_count.store(0);
        m_execute_count.store(0);
    }

    friend inline std::ostream & operator << (
            std::ostream & os,
            const TaskStats & stats)
    {
        return os << stats.m_execute_count.load() << " executed / "
                  << stats.m_schedule_count.load() << " scheduled";
    }
};
static TaskStats g_merge_stats;
static TaskStats g_enforce_stats;
static TaskStats g_sample_stats;
static TaskStats g_cleanup_stats;


void set_thread_count (size_t worker_count)
{
    POMAGMA_ASSERT_LE(1, worker_count);
    g_worker_count = worker_count;
}

void reset_stats ()
{
    g_merge_stats.clear();
    g_enforce_stats.clear();
    g_sample_stats.clear();
    g_cleanup_stats.clear();
}

void log_stats ()
{
    POMAGMA_INFO("merge tasks: " << g_merge_stats);
    POMAGMA_INFO("enforce tasks: " << g_enforce_stats);
    POMAGMA_INFO("sample tasks: " << g_sample_stats);
    POMAGMA_INFO("cleanup tasks: " << g_cleanup_stats);
}

template<class Task>
class TaskQueue
{
    tbb::concurrent_queue<Task> m_queue;

public:

    void push (const Task & task)
    {
        m_queue.push(task);
        g_working_condition.notify_one();
        g_enforce_stats.schedule();
    }

    bool try_execute ()
    {
        SharedMutex::SharedLock lock(g_strict_mutex);
        Task task;
        if (m_queue.try_pop(task)) {
            execute(task);
            g_enforce_stats.execute();
            return true;
        } else {
            return false;
        }
    }

    void cancel_referencing (Ob ob)
    {
        for (size_t i = 0, I = m_queue.unsafe_size(); i != I; ++i) {
            Task task;
            m_queue.try_pop(task);
            if (not task.references(ob)) {
                m_queue.push(task);
            }
        }
    }
};

void cancel_tasks_referencing (Ob ob);

template<>
class TaskQueue<MergeTask>
{
    tbb::concurrent_queue<MergeTask> m_queue;
    std::atomic<unsigned> m_waiting_count;

public:

    TaskQueue () : m_waiting_count(0) {}

    void push (const MergeTask & task)
    {
        m_queue.push(task);
        g_working_condition.notify_one();
        g_merge_stats.schedule();
    }

    bool try_execute ()
    {
        MergeTask task;
        if (m_queue.try_pop(task)) {
            m_waiting_count++;
            SharedMutex::UniqueLock lock(g_strict_mutex);
            m_waiting_count--;
            execute(task);
            g_merge_stats.execute();
            cancel_tasks_referencing(task.dep);
            return true;
        } else {
            return false;
        }
    }

    bool waiting () const
    {
        return m_waiting_count.load(std::memory_order_acquire);
    }
};


static TaskQueue<MergeTask> g_merge_tasks;
static TaskQueue<ExistsTask> g_exists_tasks;
static TaskQueue<PositiveOrderTask> g_positive_order_tasks;
static TaskQueue<NegativeOrderTask> g_negative_order_tasks;
static TaskQueue<NullaryFunctionTask> g_nullary_function_tasks;
static TaskQueue<InjectiveFunctionTask> g_injective_function_tasks;
static TaskQueue<BinaryFunctionTask> g_binary_function_tasks;
static TaskQueue<SymmetricFunctionTask> g_symmetric_function_tasks;
static TaskQueue<AssumeTask> g_assume_tasks;

inline void cancel_tasks_referencing (Ob ob)
{
    g_exists_tasks.cancel_referencing(ob);
    g_nullary_function_tasks.cancel_referencing(ob);
    g_injective_function_tasks.cancel_referencing(ob);
    g_binary_function_tasks.cancel_referencing(ob);
    g_symmetric_function_tasks.cancel_referencing(ob);
    g_positive_order_tasks.cancel_referencing(ob);
    g_negative_order_tasks.cancel_referencing(ob);
}

inline bool enforce_tasks_try_execute (bool cleanup)
{
    if (g_exists_tasks.try_execute() or
        g_nullary_function_tasks.try_execute() or
        g_injective_function_tasks.try_execute() or
        g_binary_function_tasks.try_execute() or
        g_symmetric_function_tasks.try_execute() or
        g_positive_order_tasks.try_execute() or
        g_negative_order_tasks.try_execute())
    {
        if (cleanup and g_worker_count > 1) {
            cleanup_tasks_push_all();
        }
        return true;
    } else {
        return false;
    }
}

inline bool sample_tasks_try_execute (rng_t & rng)
{
    SampleTask task;
    if (sample_tasks_try_pop(task)) {
        SharedMutex::SharedLock lock(g_strict_mutex);
        execute(task, rng);
        g_sample_stats.execute();
        return true;
    } else {
        return false;
    }
}

inline bool cleanup_tasks_try_execute ()
{
    CleanupTask task;
    if (cleanup_tasks_try_pop(task)) {
        SharedMutex::SharedLock lock(g_strict_mutex);
        execute(task);
        g_cleanup_stats.execute();
        return true;
    } else {
        return false;
    }
}

bool try_initialize_work (rng_t &)
{
    return g_merge_tasks.try_execute()
        or enforce_tasks_try_execute(false)
        or g_assume_tasks.try_execute()
        or cleanup_tasks_try_execute();
}

bool try_survey_work (rng_t & rng)
{
    return g_merge_tasks.try_execute()
        or enforce_tasks_try_execute(true)
        or sample_tasks_try_execute(rng)
        or cleanup_tasks_try_execute();
}

void do_work (bool (*try_work)(rng_t &))
{
    std::random_device device;
    rng_t rng(device());

    g_working_flag.store(true);
    while (likely(g_working_flag.load())) {
        ++g_working_count;
        while (try_work(rng)) {}
        if (unlikely(--g_working_count)) {
            std::unique_lock<std::mutex> lock(g_working_mutex);
            // HACK the timeout should be longer, but something is broken...
            //const auto timeout = std::chrono::seconds(60);
            const auto timeout = std::chrono::milliseconds(100);
            g_working_condition.wait_for(lock, timeout);
        } else {
            g_working_flag.store(false);
            g_working_condition.notify_all();
        }
    }
}

void initialize (const char * theory_file)
{
    insert_nullary_functions();
    if (theory_file) {
        assume_core_facts(theory_file);
    }
    cleanup_tasks_push_all();
    POMAGMA_INFO("starting " << g_worker_count << " initialize threads");
    reset_stats();
    std::vector<std::thread> threads;
    for (size_t i = 0; i < g_worker_count; ++i) {
        threads.push_back(std::thread(do_work, try_initialize_work));
    }
    for (auto & thread : threads) {
        thread.join();
    }
    POMAGMA_INFO("finished " << g_worker_count << " initialize threads");
    log_stats();
}

void survey ()
{
    POMAGMA_INFO("starting " << g_worker_count << " survey threads");
    reset_stats();
    std::vector<std::thread> threads;
    for (size_t i = 0; i < g_worker_count; ++i) {
        threads.push_back(std::thread(do_work, try_survey_work));
    }
    for (auto & thread : threads) {
        thread.join();
    }
    POMAGMA_INFO("finished " << g_worker_count << " survey threads");
    log_stats();
}

} // namespace Scheduler


void schedule (const MergeTask & task)
{
    Scheduler::g_merge_tasks.push(task);
}

void schedule (const ExistsTask & task)
{
    Scheduler::g_exists_tasks.push(task);
}

void schedule (const PositiveOrderTask & task)
{
    Scheduler::g_positive_order_tasks.push(task);
}

void schedule (const NegativeOrderTask & task)
{
    Scheduler::g_negative_order_tasks.push(task);
}

void schedule (const NullaryFunctionTask & task)
{
    Scheduler::g_nullary_function_tasks.push(task);
}

void schedule (const InjectiveFunctionTask & task)
{
    Scheduler::g_injective_function_tasks.push(task);
}

void schedule (const BinaryFunctionTask & task)
{
    Scheduler::g_binary_function_tasks.push(task);
}

void schedule (const SymmetricFunctionTask & task)
{
    Scheduler::g_symmetric_function_tasks.push(task);
}

void schedule (const AssumeTask & task)
{
    Scheduler::g_assume_tasks.push(task);
}

} // namespace pomagma
