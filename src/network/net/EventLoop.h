#pragma once 
#include "network/base/NetWork.h"
#include "network/net/TimingWheel.h"
#include "sys/epoll.h"
#include "Event.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <functional>
#include <queue>
#include <mutex>
namespace tmms{
    namespace network{
        class Event;
        class PipeEvent;
            using Func = std::function<void()>;
            using EventPtr = std::shared_ptr<Event>;
            const int kReadEvent = (EPOLLIN|EPOLLPRI|EPOLLET);
            const int kWriteEvent = (EPOLLOUT|EPOLLET);
        class EventLoop{
            public:
                EventLoop();
                ~EventLoop();
                
                void Loop();
                void Quit();
                void AddEvent(const EventPtr & event);
                void DelEvent(const EventPtr & event);
                bool EnableEventWriting(const EventPtr & event,bool enable);
                bool EnableEventReading(const EventPtr & event,bool enable);
                bool AssertInLoopThread();
                void RunInLoop(const Func & f);
                void RunInLoop(Func && f);
                bool IsInLoopThread();
                void WakeUp();
                void InsertEntry(uint32_t delag,EntryPtr entry);
                void RunAfter(double delay,const Func & f);
                void RunEvery(double delay,const Func & f);
                void RunAfter(double delay,Func && f);
                void RunEvery(double delay,Func && f);    
            private:
                TimingWheel wheels_;
                void RunFunctions();
                bool looping{false};
                int epoll_fd_{-1};
                std::vector<struct epoll_event> epoll_events_;
                std::unordered_map<int,EventPtr> event_;
                std::queue<Func> functions_;
                std::mutex lock_;
                std::shared_ptr<PipeEvent> pipe_event_;
                //PipeEventPtr pipe_event_;
        };
    }
}