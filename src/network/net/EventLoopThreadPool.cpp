#include"network/net/EventLoopThreadPool.h"
using namespace tmms::network;
namespace{
    void bind_cpu(std::thread &t,int n){
        cpu_set_t cpu;
        
        CPU_ZERO(&cpu);
        CPU_SET(n,&cpu);

        pthread_setaffinity_np(t.native_handle(),sizeof(cpu),&cpu);
    }
}
EventLoopThreadPool::EventLoopThreadPool(int thread_num,int start,int cpu){
    if(thread_num<=0){
        thread_num=1;
    }
    for(int i=0;i<thread_num;i++){
        threads_.emplace_back(std::make_shared<EventLoopThread>());
        if(cpu>0){
            int n=(start+i)%cpu;
            bind_cpu(threads_.back()->GetThread(),n);
        }
    }
}

EventLoopThreadPool::~EventLoopThreadPool(){

}

std::vector<EventLoop *> EventLoopThreadPool::GetLoop(){
    std::vector<EventLoop *> result;
    for(auto & i :threads_){
        result.emplace_back(i->Loop());
    }
    return result;
}

EventLoop *EventLoopThreadPool::GetNextLoop(){
    int index = loop_index;
    loop_index++;
    return threads_[index%threads_.size()]->Loop();
}

size_t EventLoopThreadPool::Size(){
    return threads_.size();
}

void EventLoopThreadPool::Start(){
    for(auto & i :threads_){
        i->Run();
    }
}