#include "network/TcpServer.h"
#include "network/base/NetWork.h"
using namespace tmms::network;
TcpServer::TcpServer(EventLoop * loop,const InetAddress & addr):loop_(loop),addr_(addr){
    acceptor_ = std::make_shared<Acceptor>(loop,addr);
}   

TcpServer::~TcpServer(){

}

void TcpServer::SetNewConnectionCallBack(const NewConnectionCallBack &cb){
    new_connect_cb = cb;
}

void TcpServer::SetNewConnectionCallBack(NewConnectionCallBack &&cb){
    new_connect_cb = cb;
}

void TcpServer::SetDestroyConnectionCallBack(const DestroyConnectionCallBack &cb){
    destroy_connect_cb = cb;
}   

void TcpServer::SetDestroyConnectionCallBack(DestroyConnectionCallBack &&cb){
    destroy_connect_cb = cb;
}

void TcpServer::OnAccet(int fd,const InetAddress & addr){
    NET_TRACE <<"new connect fd"<<fd<<"host:"<<addr.ToIpPort();
    TcpConnectionPtr con= std::make_shared<TcpConnection>(loop_,fd,addr_,addr);
    con->SetCloseCallBack(std::bind(&TcpServer::OnConnectionClose,this,std::placeholders::_1));
    if(write_cp_cb){
        con->SetWriteCompleteCallBack(write_cp_cb);
    }
    if(active_cb){
        con->SetActiveCallBack(active_cb);
    }
    con->SetRecvMsgCallBack(message_cb);
    connections_.insert(con);
    loop_->AddEvent(con);
    con->EnableCheckIdleTimeOut(30);
    con->EnableReading(true); 
    con->EnableCheckIdleTimeOut(30);
    if(new_connect_cb){
        new_connect_cb(con);
    }
}

void TcpServer::OnConnectionClose(const TcpConnectionPtr &con){
    NET_TRACE <<"host:"<<con->GetPeerAddr().ToIpPort()<<" close";
    loop_->DelEvent(con);
    connections_.erase(con);
    if(destroy_connect_cb){
        destroy_connect_cb(con);
    }
}

void TcpServer::SetActiveCallBack(const ActiveCallBack &cb){
    active_cb = cb;
}

void TcpServer::SetActiveCallBack(ActiveCallBack &&cb){
    active_cb = cb;
}

void TcpServer::SetMessageCallBack(const MessageCallBack &cb){
    message_cb = cb;
}

void TcpServer::SetMessageCallBack(MessageCallBack &&cb){
    message_cb = cb;
}

void TcpServer::SetWriteCompleteCallBack(const WriteCompleteCallBack &cb){
    write_cp_cb = cb;
}

void TcpServer::SetWriteCompleteCallBack(WriteCompleteCallBack &&cb){
    write_cp_cb = cb;
}

void TcpServer::Start(){
    acceptor_->SetAcceptorCallBack(std::bind(&TcpServer::OnAccet,this,std::placeholders::_1,std::placeholders::_2));
    acceptor_->Start();
}

void TcpServer::Stop(){
    acceptor_->Stop();
}


