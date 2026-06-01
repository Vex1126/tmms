#include"network/net/PipeEvent.h"
#include"network/base/NetWork.h"
#include<unistd.h>
#include<fcntl.h>
#include<iostream>
using namespace tmms::network;
PipeEvent::PipeEvent(EventLoop * loop):Event(loop){
    int fd[2]={0,};
    int ret = pipe2(fd,O_NONBLOCK);
    if(ret<0){
        NET_ERROR << "pipe2 failed";
        exit(-1);
    }
    fd_=fd[0];
    write_fd=fd[1];
}

PipeEvent::~PipeEvent(){
    if(write_fd){
        ::close(write_fd);
        write_fd=-1;
    }
}

void PipeEvent::OnRead(){
    int64_t tmp;
    int ret = ::read(fd_,&tmp,sizeof(tmp));
    if(ret<0){
        NET_ERROR << "read failed";
        return;
    }
    std::cout<<"read success tmp:"<<tmp<<std::endl;
}

void PipeEvent::OnClose(){
    if(write_fd){
        ::close(write_fd);
        write_fd=-1;
    }
}

void PipeEvent::OnError(const std::string &msg){
    std::cout<<"error "<<msg<<std::endl;
}

void PipeEvent::Write(const char * data,size_t len){
    int ret = ::write(write_fd,data,len);
    if(ret<0){
        NET_ERROR<<"write failed";
    }
}