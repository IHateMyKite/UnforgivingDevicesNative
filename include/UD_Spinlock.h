#pragma once

namespace
{
    class Spinlock
    {
    public:
        void Lock()
        {
            while (_lock) {_cnt++;}
            _lock = true;
        }
        void Unlock()
        {
            _lock = false;
        }
    private:
        mutable bool        _lock   = false;
        mutable uint64_t    _cnt    = 0U; //use counter, so the compiler doesnt ignore the while loop, because of optimization
    };

    class UniqueLock
    {
    public:
        UniqueLock(Spinlock& a_lock)
        {
            _lock = &a_lock;
            _lock->Lock();
        }
        ~UniqueLock()
        {
            _lock->Unlock();
        }
    private:
        mutable Spinlock* _lock;
    };
}