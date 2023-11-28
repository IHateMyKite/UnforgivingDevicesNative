#pragma once

namespace
{
    class Spinlock
    {
    public:
        void Lock()
        {
            while (_lock) {;}
            _lock = true;
        }
        void Unlock()
        {
            _lock = false;
        }
    private:
        mutable bool _lock = false;
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