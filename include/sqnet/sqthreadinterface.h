#ifndef SNQU_THREAD_INTERFACE_H
#define SNQU_THREAD_INTERFACE_H

namespace snqu{ 

    template <typename Task>
    class ThreadInterface
    {
    public:
        typedef ThreadInterface<Task> TaskManager_ptr;
        ThreadInterface(){};
        ~ThreadInterface(){};

        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void post_task(Task) = 0;
        virtual void set_timer_task(uint32_t id, uint32_t second, std::function<void(uint32_t)> proc) = 0;
    };

}

#endif