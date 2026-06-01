#include"mmedia/rtmp/RtmpServer.h"
using namespace tmms::mm;
RtmpServer::RtmpServer(EventLoop * loop,const InetAddress & local,RtmpHandler *rtmphander):
TcpServer(loop,local),rtmp_hander_(rtmphander){

}

RtmpServer::~RtmpServer(){

}

void RtmpServer::Start(){
    TcpServer::SetNewConnectionCallBack(std::bind(&RtmpServer::OnNewConnection,this,std::placeholders::_1));
    TcpServer::SetDestroyConnectionCallBack(std::bind(&RtmpServer::OnDestroyed,this,std::placeholders::_1));
    TcpServer::SetMessageCallBack(std::bind(&RtmpServer::OnMessage,this,std::placeholders::_1,std::placeholders::_2)); 
    TcpServer::SetWriteCompleteCallBack(std::bind(&RtmpServer::OnWriteComplete,this,std::placeholders::_1));  
    TcpServer::SetActiveCallBack(std::bind(&RtmpServer::OnActive,this,std::placeholders::_1));  
    TcpServer::Start();
}

void RtmpServer::Stop(){
    TcpServer::Stop();
}

void RtmpServer::OnNewConnection(const TcpConnectionPtr & con){
    if(rtmp_hander_){
        rtmp_hander_->OnNewConnection(con);
    }
    RtmpContextPtr rtmp_context = std::make_shared<RtmpContext>(con,rtmp_hander_);
    con->SetContext(kRtmpContext,rtmp_context);
    rtmp_context->StartHandShake();
}

void RtmpServer::OnDestroyed(const TcpConnectionPtr & con){
    if(rtmp_hander_){
        rtmp_hander_->OnConnectionDestroy(con);
    }
    con->ClearContext(kRtmpContext);
}

void RtmpServer::OnMessage(const TcpConnectionPtr & con,MsgBuffer & buf){
    RtmpContextPtr shake = con->GetContext<RtmpContext>(kRtmpContext);
    if(!shake){
        RTMP_ERROR << "host:" << con->GetPeerAddr().ToIpPort() << " RtmpContext is null";
        return;
    }
    int ret = shake->Parse(buf);
    if(ret == 0){

    }
    else if(ret == -1){
        RTMP_ERROR << "host:" << con->GetPeerAddr().ToIpPort() << " Parse returned -1, closing connection";
        con->ForceClose();
    }

}

void RtmpServer::OnWriteComplete(const ConnectionPtr & con){
    RtmpContextPtr shake = con->GetContext<RtmpContext>(kRtmpContext);
    shake->WriteComplete();
}

void RtmpServer::OnActive(const ConnectionPtr & con){
    if(rtmp_hander_){
        rtmp_hander_->OnActive(con);
    }    
}