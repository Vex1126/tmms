#include"network/net/Event.h"
#include"network/net/EventLoop.h"
#include"network/net/EventLoopThread.h"
#include"network/net/EventLoopThreadPool.h"
#include"base/Singleton.h"
#include<iostream>
#include<vector>
using namespace tmms::base;
using namespace tmms::network;

void test(){
    auto pool = EventLoopThreadPool::Instance(4);
    std::cout<<"thread id is "<<std::this_thread::get_id()<<std::endl;
    pool->Start();
    EventLoop * lp=pool->GetNextLoop();
    lp->RunEvery(1,[](){
        std::cout << "1 second test"<<std::endl; 
    });
    lp->RunEvery(1,[](){
        std::cout << "1 second test"<<std::endl; 
    });
    lp->RunEvery(5,[](){
        std::cout << "5 second test"<<std::endl; 
    });
}

int main(int argc, char const *argv[])
{
    test();
    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(1));        
    }
    return 0;
}
