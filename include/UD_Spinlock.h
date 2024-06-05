#pragma once

namespace Utils
{
    class Spinlock
    {
    public:
        void Lock()
        {
            while (_lock.exchange(true))
            {
                //DEBUG("Lock - Waiting for free - {:016X} - {}",(uintptr_t)this,_lock)
                ;
            }
        }
        void Unlock()
        {
            _lock.store(false);
        }
    private:
        mutable std::atomic<bool> _lock   = false;
    };

    class UniqueLock
    {
    public:
        UniqueLock(Spinlock& a_lock)
        {
            _lock = &a_lock;
            //DEBUG("UniqueLock - {:016X} - {:016X}",(uintptr_t)this,(uintptr_t)_lock)
            _lock->Lock();
        }
        ~UniqueLock()
        {
            //DEBUG("~UniqueLock - {:016X} - {:016X}",(uintptr_t)this,(uintptr_t)_lock)
            _lock->Unlock();
        }
    private:
        mutable Spinlock* _lock;
    };
}