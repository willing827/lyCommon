#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <future>
#include <mutex>
#include <sqstd/sqtaskqueue.h>

namespace snqu {

    class TaskThreadPool 
    {

    public:
        TaskThreadPool(int nThreads=2);

        // the destructor waits for all the functions in the queue to be finished
        ~TaskThreadPool();

        // get the number of running threads in the pool
        int size();

        // number of idle threads
        int idle_num();

        std::thread & get_thread(int i);

        // change the number of threads in the pool
        // should be called from one thread, otherwise be careful to not interleave, also with this->stop()
        // nThreads must be >= 0
        void resize(int nThreads);

        std::function<void(int)> pop();

        void stop(bool isWait = false);

        template<typename F, typename... Rest>
        auto push(F && f, Rest&&... rest)->std::future<decltype(f(0, rest...))> 
        {
            auto pck = std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>(
                std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Rest>(rest)...)
                );
            auto _f = new std::function<void(int id)>([pck](int id) 
            {
                (*pck)(id);
            });
            this->m_queue.push(_f);
            std::unique_lock<std::mutex> lock(this->m_mutex);
            this->m_cv.notify_one();
            return pck->get_future();
        }

        template<typename Func>
        auto push(Func && f)->std::future<decltype(f(0))> 
        {
            auto pck = std::make_shared<std::packaged_task<decltype(f(0))(int)>>(std::forward<Func>(f));
            auto _f = new std::function<void(int id)>([pck](int id) 
            {
                (*pck)(id);
            });
            this->m_queue.push(_f);
            std::unique_lock<std::mutex> lock(this->m_mutex);
            this->m_cv.notify_one();
            return pck->get_future();
        }

    private:
        // deleted
        TaskThreadPool(const TaskThreadPool &);// = delete;
        TaskThreadPool(TaskThreadPool &&);// = delete;
        TaskThreadPool & operator=(const TaskThreadPool &);// = delete;
        TaskThreadPool & operator=(TaskThreadPool &&);// = delete;

        void set_thread(int i);
        void init();

        std::vector<std::unique_ptr<std::thread>> threads;
        std::vector<std::shared_ptr<std::atomic<bool>>> flags;
        snqu::TaskQueue<std::function<void(int id)>*> m_queue;
        std::atomic<bool> isDone;
        std::atomic<bool> isStop;
        std::atomic<int> nWaiting;  // how many threads are waiting

        std::mutex m_mutex;
        std::condition_variable m_cv;
    };

}

#endif // _THREAD_POOL_H