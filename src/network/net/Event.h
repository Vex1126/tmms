#pragma once 
#include"EventLoop.h"
#include<string>
#include<unistd.h>
#include<memory>
namespace tmms{
    namespace network{
        class EventLoop;
         
        class Event:public std::enable_shared_from_this<Event>{
                friend class EventLoop;
            public:
                Event(EventLoop * loop);
                virtual ~Event();
                Event(EventLoop * loop,int fd);

                virtual void OnRead(){};
                virtual void OnWrite(){};
                virtual void OnClose(){};
                virtual void OnError(const std::string &msg){};
                void Close();
                bool EnableWriting(bool enable);
                bool EnableReading(bool enable);
                int Fd()const;
            protected:
                EventLoop * loop_{nullptr};
                int fd_{-1};
                int event_{0};
        };
    }
}