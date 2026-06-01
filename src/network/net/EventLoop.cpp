#include"network/net/EventLoop.h"
#include"network/net/PipeEvent.h"
#include"base/TTime.h"
using namespace tmms::network;
static thread_local EventLoop* t_local_eventloop = nullptr;
EventLoop::EventLoop():epoll_fd_(epoll_create(1024)),epoll_events_(1024){
    t_local_eventloop = this; 
    if(!pipe_event_){
        pipe_event_ = std::make_shared<PipeEvent>(this);
    }
    AddEvent(pipe_event_);
}

EventLoop::~EventLoop(){
    Quit();
}

void EventLoop::Loop(){
    looping = true;
    int64_t outtime = 1000;
    while(looping){
        auto ret = epoll_wait(epoll_fd_,&epoll_events_[0],epoll_events_.size(),outtime);

        if(ret >= 0){
            for(int i=0;i<ret;i++){
                struct epoll_event &ev = epoll_events_[i];

                if(ev.data.fd<0){
                    continue;                    
                }

                auto iter = event_.find(ev.data.fd);
                if(iter ==event_.end()){
                    continue;
                }
                EventPtr &event = iter->second;
                if(ev.events&EPOLLERR){
                    int error = 0;
                    socklen_t len = sizeof(error); 
                    getsockopt(event->fd_,SOL_SOCKET,SO_ERROR,&error,&len);
                    event->OnError(strerror(error));

                }else if((ev.events&EPOLLHUP)&&!(ev.events&EPOLLIN)){
                    
                    event->OnClose();

                }else if(ev.events&(EPOLLIN | EPOLLPRI)){

                    event->OnRead();

                }else if (ev.events&EPOLLOUT)
                {
                    event->OnWrite();
                }
            }
                if(ret == epoll_events_.size()){
                    epoll_events_.resize(epoll_events_.size()*2);
                }
                RunFunctions();
                int64_t now =  tmms::base::TTime::NowMs();
                wheels_.OnTimer(now);
        }else if(ret < 0){
            NET_ERROR << "epoll wait failed";
        }
    }
}

void EventLoop::AddEvent(const EventPtr & event){
    auto iter = event_.find(event->fd_);
    if(iter != event_.end()){
        return;
    }
    event_[event->fd_] = event;
    event->event_ |= kReadEvent;
    struct epoll_event ev;
    memset(&ev,0x00,sizeof(ev));
    ev.events = event->event_;
    ev.data.fd = event->fd_;
    epoll_ctl(epoll_fd_,EPOLL_CTL_ADD,event->fd_,&ev);
}

void EventLoop::DelEvent(const EventPtr & event){
    auto iter = event_.find(event->fd_);
    if(iter == event_.end()){
        return;
    }
    event_.erase(event->fd_);
    struct epoll_event ev;
    memset(&ev,0x00,sizeof(ev));
    ev.events = event->event_;
    ev.data.fd = event->fd_;
    epoll_ctl(epoll_fd_,EPOLL_CTL_DEL,event->fd_,&ev);
}

bool EventLoop::EnableEventWriting(const EventPtr & event,bool enable){
    auto iter = event_.find(event->fd_);
    if(iter == event_.end()){
        NET_ERROR << "can not find fd";
        return false;
    }

    if(enable){
        event->event_ |= kWriteEvent;
    }else{
        event->event_ &= ~kWriteEvent;
    }
    struct epoll_event ev;
    memset(&ev,0x00,sizeof(ev));
    ev.events = event->event_;
    ev.data.fd = event->fd_;
    epoll_ctl(epoll_fd_,EPOLL_CTL_MOD,event->fd_,&ev);
    return true;
}

bool EventLoop::EnableEventReading(const EventPtr & event,bool enable){
    auto iter = event_.find(event->fd_);
    if(iter == event_.end()){
        NET_ERROR << "can not find fd";
        return false;
    }

    if(enable){
        event->event_ |= kReadEvent;
    }else{
        event->event_ &= ~kReadEvent;
    }
    struct epoll_event ev;
    memset(&ev,0x00,sizeof(ev));
    ev.events = event->event_;
    ev.data.fd = event->fd_;
    epoll_ctl(epoll_fd_,EPOLL_CTL_MOD,event->fd_,&ev);
    return true;
}

void EventLoop::Quit(){
    looping = false;
}

bool EventLoop::AssertInLoopThread(){
    if(!IsInLoopThread()){
        NET_ERROR << "IsInLoopThread is false";
        return false;
    }
    return true;
}

bool EventLoop::IsInLoopThread(){
    return t_local_eventloop==this;
}

void EventLoop::RunInLoop(const Func & f){
    if(IsInLoopThread()){
        f();
    }else{
        std::lock_guard<std::mutex>lock(lock_);
        functions_.push(f);
        WakeUp();
    }
}

void EventLoop::RunInLoop(Func && f){
    if(IsInLoopThread()){
        f();
    }else{
        std::lock_guard<std::mutex>lock(lock_);
        functions_.push(std::move(f));
        WakeUp();
    }
}

void EventLoop::RunFunctions(){
    std::lock_guard<std::mutex>lock(lock_);
    while(!functions_.empty()){
        auto &f = functions_.front();
        f();
        functions_.pop();
    }
}

void EventLoop::WakeUp(){
    int64_t tmp=1;
    pipe_event_->Write((const char *)&tmp,sizeof(tmp));
}

void EventLoop::InsertEntry(uint32_t delay,EntryPtr entry){
    if(IsInLoopThread()){
        wheels_.InsertEntry(delay,entry);
    }else{
        RunInLoop([this,delay,entry](){
           wheels_.InsertEntry(delay,entry); 
        });
    }
}

void EventLoop::RunAfter(double delay,const Func & f){
    if(IsInLoopThread()){
        wheels_.RunAfter(delay,f);
    }else{
        RunInLoop([this,delay,f](){
           wheels_.RunAfter(delay,f);
        });
    }
}

void EventLoop::RunEvery(double delay,const Func & f){
    if(IsInLoopThread()){
        wheels_.RunEvery(delay,f);
    }else{
        RunInLoop([this,delay,f](){
           wheels_.RunEvery(delay,f);
        });
    }
}

void EventLoop::RunAfter(double delay,Func && f){
    if(IsInLoopThread()){
        wheels_.RunAfter(delay,f);
    }else{
        RunInLoop([this,delay,f](){
           wheels_.RunAfter(delay,f);
        });
    }
}

void EventLoop::RunEvery(double delay,Func && f){
    if(IsInLoopThread()){
        wheels_.RunEvery(delay,f);
    }else{
        RunInLoop([this,delay,f](){
           wheels_.RunEvery(delay,f);
        });
    }
}