#include"network/UdpClient.h"
#include"network/base/SocketOpt.h"
using namespace tmms::network;
UdpClient::UdpClient(EventLoop * loop,const InetAddress & server):UdpSocket(loop,-1,InetAddress(),server),server_addr_(server){

}

UdpClient::~UdpClient(){

}

void UdpClient::Connect(){
    loop_->RunInLoop([this](){
        ConnectInLoop();
    });
}

void UdpClient::ConnectInLoop(){
    loop_->AssertInLoopThread();
    fd_ = SocketOpt::CreateNonblockingUdpSocket(AF_INET);
    if(fd_<0){
        NET_ERROR <<"fd_ create fail";
        OnClose();
        return;
    }
    connected_=true;
    server_addr_.GetSockAddr((sockaddr*)&sock_addr_);
    loop_->AddEvent(std::dynamic_pointer_cast<UdpClient>(shared_from_this()));
    SocketOpt socket(fd_);
    socket.Connect(server_addr_);
    if(connect_cb_){
        connect_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()),true);
    }

}

void UdpClient::SetConnectCallBack(const ConnectCallBack & cb){
    connect_cb_ = cb;
}

void UdpClient::SetConnectCallBack(ConnectCallBack && cb){
    connect_cb_ = std::move(cb);
}

void UdpClient::Send(const char * buf,size_t size){
    UdpSocket::Send(buf,size,(sockaddr*)&sock_addr_,len);
}

void UdpClient::Send(std::list<UdpBufferNodePtr>&list){

}

void UdpClient::OnClose(){
    if(connected_){
        loop_->DelEvent(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
        connected_ = false;
        UdpSocket::Close();
    }   
}