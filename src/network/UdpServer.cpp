#include"network/base/NetWork.h"
#include"network/UdpServer.h"
#include"network/base/SocketOpt.h"
using namespace tmms::network;
UdpServer::UdpServer(EventLoop * loop,const InetAddress & server_addr):UdpSocket(loop,-1,server_addr,InetAddress()),server_addr_(server_addr){

}

UdpServer::~UdpServer(){
    Stop();
}

void UdpServer::Start(){
    loop_->RunInLoop([this](){
        Open();
    });
}

void UdpServer::Stop(){
    loop_->RunInLoop([this](){
        loop_->DelEvent(std::dynamic_pointer_cast<UdpServer>(shared_from_this()));
        OnClose();
    });   
}

void UdpServer::Open(){
    loop_->AssertInLoopThread();
    fd_ = SocketOpt::CreateNonblockingUdpSocket(AF_INET);
    if(fd_<0){
        NET_ERROR <<"fd create failed";
        OnClose();
        return;
    }
    loop_->AddEvent(std::dynamic_pointer_cast<UdpServer>(shared_from_this()));
    SocketOpt opt(fd_);
    opt.BindAddress(server_addr_);
}   