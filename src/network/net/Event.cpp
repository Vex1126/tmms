#include"Event.h"
using namespace tmms::network;
Event::Event(EventLoop * loop):loop_(loop){

}
Event::~Event(){
    Close();
}

Event::Event(EventLoop * loop,int fd):loop_(loop),fd_(fd){

}


bool Event::EnableWriting(bool enable){
    loop_->EnableEventWriting(shared_from_this(),enable);
    return true;
}

bool Event::EnableReading(bool enable){
    loop_->EnableEventReading(shared_from_this(),enable);
    return true;
}

int Event::Fd()const{
    return fd_;
}

void Event::Close(){
    if(fd_){
        ::close(fd_);
        fd_=-1;
    }    
}