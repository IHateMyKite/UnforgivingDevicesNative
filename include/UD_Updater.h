#pragma once

namespace UD 
{

    struct SerialTask
    {
        void*           arg;
        std::function<void(void*)> task;
        bool            freearg;
    };



    class UpdateManager
    {
    SINGLETONHEADER(UpdateManager)
    public:
        void CreateUpdateThreads(void);
    public:
        void UpdateThread1(const float& a_delta);   //used for slow update
        void UpdateThread2(const float& a_delta);   //used for fps tied updated (is called on every frame). Should be fast

		void Hook();
		
        static void ActorUpdatePatched(RE::Actor* a_this, float a_delta);
        REL::Relocation<decltype(ActorUpdatePatched)> ActorUpdate;

        bool AddSerTask(std::function<void(void*)> a_task, void* a_arg, bool a_freearg);
    private:
        int _installed = false;

        std::mutex _taskmutex;
        void CallSerTasks();
        std::vector<SerialTask> _taskstack;
    };
};