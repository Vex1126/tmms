#include "EventLoopThread.h"
using namespace tmms::network;
EventLoopThread::EventLoopThread():thread_([this](){StartEventLoop();}){

}

EventLoopThread::~EventLoopThread(){
    Run();
    if(loop_){
        loop_->Quit();
    }

    if(thread_.joinable()){
        thread_.join();
    }
}

EventLoop * EventLoopThread::Loop(){
    return loop_;
}

void EventLoopThread::Run(){
    std::call_once(once_,[this](){
        {
            std::lock_guard<std::mutex> lock(lock_);
            running_=true;
            cond_.notify_all();
        }
        auto f=promise_loop.get_future();
        f.get();
    });
}

void EventLoopThread::StartEventLoop(){
    EventLoop loop;
    std::unique_lock<std::mutex> lock(lock_);
    cond_.wait(lock,[this](){
        return running_;
    });
    loop_ = &loop;
    promise_loop.set_value(1);
    loop.Loop();
    loop_=nullptr;
}

std::thread &EventLoopThread::GetThread(){
    return thread_;
}