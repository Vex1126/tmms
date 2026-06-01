#pragma once
#include"Task.h"
#include<unordered_set>
#include<mutex>
#include "Singleton.h"
using namespace tmms::base;
namespace tmms{
    namespace base{
        class TaskMgr:public Singleton<TaskMgr>{
            public:
                TaskMgr()=default;
                ~TaskMgr()=default;
                void DoWork();
                void Add(TaskPtr & task);
                void Del(TaskPtr & task);
            private:
                std::unordered_set<TaskPtr> tasks_;
                std::mutex lock_;
        };
    }
    #define sTaskMgr tmms::base::Singleton<tmms::base::TaskMgr>::Instance()
}