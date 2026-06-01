#include"network/TcpClient.h"
#include<errno.h>
using namespace tmms::network;
TcpClient::TcpClient(EventLoop * loop,const InetAddress &addr):TcpConnection(loop,-1,InetAddress(),addr),sever_addr_(addr){

}

TcpClient::~TcpClient(){
    OnClose();
}

void TcpClient::SetConnectCallBack(const ConnectCallBack & cb){
    connect_cb = cb;
}
void TcpClient::SetConnectCallBack(ConnectCallBack && cb){
    connect_cb = cb;
}
void TcpClient::Connect(){
    loop_->RunInLoop([this](){
        ConnectInLoop();
    });
}
void TcpClient::ConnectInLoop(){
    loop_->AssertInLoopThread();
    fd_ = SocketOpt::CreateNonblockingTcpSocket(AF_INET);
    if(fd_ < 0){
        OnClose();
        return;
    }
    statu_ = kTcpConStatuConnecting;
    loop_->AddEvent(std::dynamic_pointer_cast<TcpClient>(shared_from_this()));
    EnableWriting(true);
    SocketOpt opt(fd_);
    auto ret = opt.Connect(sever_addr_);
    if(ret == 0){
        UpdateConnectionStatu();
        return;
    }else if(ret == -1){
        if(errno != EINPROGRESS){
            NET_ERROR <<"connect server:"<<sever_addr_.ToIpPort()<<" error, errno="<<errno;
            OnClose();
            return;
        }
    }
}

void TcpClient::UpdateConnectionStatu(){
        statu_ = kTcpConStatuConnected;
        EnableWriting(false);
        EnableReading(true);
        if(connect_cb){
            connect_cb(std::dynamic_pointer_cast<TcpClient>(shared_from_this()),true);
        }    
}

bool TcpClient::CheckError(){
    int error = 0;
    socklen_t len = sizeof(error);
    ::getsockopt(fd_,SOL_SOCKET,SO_ERROR,&error,&len);
    return error!=0;
}

void TcpClient::OnRead(){
    if(statu_ == kTcpConStatuConnecting){
        if(CheckError()){
            NET_ERROR <<"connect server:"<<sever_addr_.ToIpPort()<<"error"<<errno;
            OnClose();
            return;        
        }
        UpdateConnectionStatu();
    }
    
    if(statu_ == kTcpConStatuConnected){
        
        TcpConnection::OnRead();
    }
}

void TcpClient::OnWrite(){
    if(statu_ == kTcpConStatuConnecting){
        if(CheckError()){
            NET_ERROR <<"connect server:"<<sever_addr_.ToIpPort()<<"error"<<errno;
            OnClose();
            return;        
        }
        UpdateConnectionStatu();
    }
    if(statu_ == kTcpConStatuConnected){
        TcpConnection::OnWrite();
    }
}

void TcpClient::OnClose(){
    if(statu_ == kTcpConStatuConnecting || statu_ == kTcpConStatuConnected){
        loop_->DelEvent(std::dynamic_pointer_cast<TcpClient>(shared_from_this()));
    }
    statu_ = kTcpConStatuDisConnected;
    TcpConnection::OnClose();
}

void TcpClient::Send(const char * buf,size_t len){
    if(statu_ == kTcpConStatuConnected){
        TcpConnection::Send(buf,len);
    }
}

void TcpClient::Send(std::list<BufferNodePtr>&list){
    if(statu_ == kTcpConStatuConnected){
        TcpConnection::Send(list);
    }
}