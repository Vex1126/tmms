#include<network/net/Acceptor.h>
using namespace tmms::network;
Acceptor::Acceptor(EventLoop * loop,const InetAddress &addr):Event(loop),addr_(addr){
    
}   

Acceptor::~Acceptor(){
    Stop();
    if(socket_opt){
        delete socket_opt;
        socket_opt=nullptr;
    }
}

void Acceptor::Open(){
    if(fd_>0){
        ::close(fd_);
        fd_=-1;
    }
    if(addr_.IsIpV6()){
        fd_=SocketOpt::CreateNonblockingTcpSocket(AF_INET6);
    }else{
        fd_=SocketOpt::CreateNonblockingTcpSocket(AF_INET);
    }
    if(fd_ < 0){
        NET_ERROR <<"fd_ creater failed";
        exit(-1);
    }
    if(socket_opt){ 
        delete socket_opt;
        socket_opt=nullptr;
    }
    loop_->AddEvent(std::dynamic_pointer_cast<Acceptor>(shared_from_this()));
    socket_opt = new SocketOpt(fd_);
    socket_opt->SetReuseAddr(true);
    socket_opt->SetPortAddr(true);
    socket_opt->BindAddress(addr_);
    socket_opt->Listen();
}

void Acceptor::Start(){
    loop_->RunInLoop([this](){
        Open();
    });
}

void Acceptor::Stop(){
    loop_->DelEvent(std::dynamic_pointer_cast<Acceptor>(shared_from_this()));
}

void Acceptor::SetAcceptorCallBack(const AcceptorCallBack &cb){
    accept_cb_ = cb;
}

void Acceptor::SetAcceptorCallBack(AcceptorCallBack &&cb){
    accept_cb_ = cb;    
}

void Acceptor::OnRead(){
    if(!socket_opt){
        return;
    }
    while(true){
        InetAddress addr;
        auto sock_ = socket_opt->Accept(&addr);
        if(sock_>=0){
            if(accept_cb_){
                accept_cb_(sock_,addr);
            }
        }else{
            if(errno!=EINTR && errno!=EAGAIN){
                NET_ERROR <<"accept failed";    
                OnClose();                    
            }
            break;
        }
    }
}

void Acceptor::OnError(const std::string &msg){
    NET_ERROR <<"accept failed";    
    OnClose();      
}

void Acceptor::OnClose(){
    Stop();
    Open();
}