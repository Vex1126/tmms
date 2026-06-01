#include"Singleton.h"
#include<iostream>
#include"TaskMgr.h"
#include<memory>
#include"TTime.h"
#include<thread>
#include<chrono>
#include"LogStream.h"
#include"FileMgr.h"
using namespace tmms::base;
void test(){
        std::cout<<"now time:"<<TTime::NowMs()<<std::endl;
    auto callback1=[](const TaskPtr task){
        std::cout<<"task1 interval="<<1000<<" now time:"<<TTime::NowMs()<<std::endl;
    };
    TaskPtr task1=std::make_shared<Task>(callback1,1000);

        auto callback2=[](const TaskPtr task){
        std::cout<<"task2 interval="<<500<<" now time:"<<TTime::NowMs()<<std::endl;
    };
    TaskPtr task2=std::make_shared<Task>(callback2,500);

        auto callback3=[](const TaskPtr task){
        std::cout<<"task3 interval="<<500<<" now time:"<<TTime::NowMs()<<std::endl;
    };
    TaskPtr task3=std::make_shared<Task>(callback3,500);

        auto callback4=[](const TaskPtr task){
        std::cout<<"task4 interval="<<2000<<" now time:"<<TTime::NowMs()<<std::endl;
    };
    TaskPtr task4=std::make_shared<Task>(callback4,2000);


    sTaskMgr->Add(task1);
    sTaskMgr->Add(task2);
    sTaskMgr->Add(task3);
    sTaskMgr->Add(task4);

    while(1){
        sTaskMgr->DoWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
static std::thread t;
void test2(){
    t = std::thread([](){
        while(true){
            LOG_TRACE << "test trace now:" <<TTime::Now();
            LOG_DEBUG << "test debug now:" <<TTime::Now();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));      
        }
    });
    t.detach();

}

int main() {
    FileLogPtr log=sFileMgr->GetFileLog("test.log");
    log->SetRotate(kRotateMinute);
    tmms::base::g_logger = tmms::base::Logger::Instance(log);
    tmms::base::g_logger->SetLogLevel(kTrace);
    auto callback4=[](const TaskPtr task){
        sFileMgr->OnCheck();
        task->Restart();
    };
    TaskPtr task4=std::make_shared<Task>(callback4,1000);
        sTaskMgr->Add(task4);
    test2();
    while(true){
        sTaskMgr->DoWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return 0;
}
