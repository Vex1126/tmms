#pragma once
#include"network/net/EventLoopThread.h"
#include"network/net/EventLoop.h"
#include"base/Singleton.h"
#include<pthread.h>
#include<memory>
#include<vector>
#include<atomic>
namespace tmms{
    namespace network{
        using EventLoopThreadPtr = std::shared_ptr<EventLoopThread>;
        class EventLoopThreadPool:public base::Singleton<EventLoopThreadPool>{
            friend class base::Singleton<EventLoopThreadPool>;
            public:
                std::vector<EventLoop *> GetLoop();
                EventLoop *GetNextLoop();
                size_t Size();
                void Start();

            private:
                EventLoopThreadPool(int thread_num,int start=0,int cpu_num=4);
                ~EventLoopThreadPool();                
                std::vector<EventLoopThreadPtr>threads_;
                std::atomic_int32_t loop_index{0};
        };
    }
}