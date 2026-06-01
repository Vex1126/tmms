#pragma once
#include "network/net/EventLoop.h"
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
namespace tmms{
    namespace network{
        class EventLoopThread{
            public: 
                EventLoopThread();
                ~EventLoopThread();
                std::thread & GetThread();
                EventLoop * Loop();
                void Run();
            private:
                void StartEventLoop();
                EventLoop * loop_{nullptr};
                std::mutex lock_;
                std::condition_variable cond_;
                bool running_{false};
                std::thread thread_;
                std::promise<int> promise_loop;
                std::once_flag once_;
        };
    }
}