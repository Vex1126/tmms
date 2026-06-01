#include"Task.h"

using namespace tmms::base;
Task::Task(const TaskCallback & cb,int64_t interval):interval_(interval),when_(TTime::NowMs()+interval),cb_(cb){

}

Task::Task(const TaskCallback && cb,int64_t interval):interval_(interval),when_(TTime::NowMs()+interval),cb_(std::move(cb)){

}

void Task::Run(){
    if(cb_){
        cb_(shared_from_this());
    }
}

void Task::Restart(){
    when_=TTime::NowMs()+interval_;
}

int64_t Task::When(){
    return when_;
}