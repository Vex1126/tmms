#include"TaskMgr.h"

void TaskMgr::DoWork(){
    std::lock_guard<std::mutex> lock(lock_);
    int64_t now=TTime::NowMs();
    for(auto iter=tasks_.begin();iter!=tasks_.end();){
        if((*iter)->When()<now){
            (*iter)->Run();
            if((*iter)->When()<now){
                iter=tasks_.erase(iter);
                continue;
            }
        }
        iter++;
    }
}

void TaskMgr::Add(TaskPtr & task){
    std::lock_guard<std::mutex> lock(lock_);
    if(tasks_.find(task)!=tasks_.end()){
        return;
    }
    tasks_.emplace(task);
}

void TaskMgr::Del(TaskPtr & task){
    std::lock_guard<std::mutex> lock(lock_);
    tasks_.erase(task);
}