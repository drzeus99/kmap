#ifndef GLOBAL_LOCK_H
#define GLOBAL_LOCK_H
#include <mutex>

//change this to use thread locks rather than booleans
class global_lock
{
    public:
        global_lock();
        ~global_lock();
        void set_lock(bool);
        bool try_lock();
        template <class T, class Function>
        void run_lock(T, Function);
    private:
        std::mutex lock;
};

template <class T, class Function>
void global_lock::run_lock(T input, Function fn)
{
	lock.lock();
	fn(input);
	lock.unlock();
}

#endif // GLOBAL_LOCK_H
