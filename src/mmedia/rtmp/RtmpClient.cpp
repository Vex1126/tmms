#include"mmedia/rtmp/RtmpClient.h"
using namespace tmms::network;
using namespace tmms::mm;
RtmpClient::RtmpClient(EventLoop * loop,RtmpHandler *handler):loop_(loop),handler_(handler){

}

RtmpClient::~RtmpClient(){

}

void RtmpClient::OnWriteComplete(const TcpConnectionPtr & con){
    auto context = con->GetContext<RtmpContext>(kRtmpContext);
        if(context){
            context->WriteComplete();
        }
}

void RtmpClient::OnConnection(const TcpConnectionPtr & con,bool connected){
    if(con){
        auto context = std::make_shared<RtmpContext>(con,handler_,true);
        if(is_playing){
            context->Play(url_);
        }else{
            context->Publish(url_);
        }
        con->SetContext(kRtmpContext,context);
        context->StartHandShake();
    }
}

void RtmpClient::OnMessage(const TcpConnectionPtr & con,MsgBuffer &msg){
    auto context = con->GetContext<RtmpContext>(kRtmpContext);
    if(context){
        auto ret =  context->Parse(msg);
        if(ret<0){
            con->ForceClose();
        }

    }
}

void RtmpClient::SetCloseCallBack(const CloseConnectionCallBack & cb){
    close_cb_ = cb;
}

void RtmpClient::SetCloseCallBack(CloseConnectionCallBack && cb){
    close_cb_ = std::move(cb);
}

void RtmpClient::Play(const std::string &url){
    is_playing = true;
    url_ = url;
    CreateTcpClient();
}

void RtmpClient::Publish(const std::string &url){
    is_playing = false;
    url_ = url;
    CreateTcpClient();
}
void RtmpClient::CreateTcpClient(){
    auto ret = ParseUrl(url_);
    if(!ret){
        if(close_cb_){
            close_cb_(nullptr);
        }
        RTMP_ERROR << "parse url failed";
    }
    tcp_client_ = std::make_shared<TcpClient>(loop_,addr_);
    tcp_client_->SetConnectCallBack(std::bind(&RtmpClient::OnConnection,this,std::placeholders::_1,std::placeholders::_2));
    tcp_client_->SetCloseCallBack(close_cb_);
    tcp_client_->SetWriteCompleteCallBack(std::bind(&RtmpClient::OnWriteComplete,this,std::placeholders::_1));
    tcp_client_->SetRecvMsgCallBack(std::bind(&RtmpClient::OnMessage,this,std::placeholders::_1,std::placeholders::_2));
    tcp_client_->Connect();
}

bool RtmpClient::ParseUrl(const std::string &url){
    // Expected format: rtmp://host[:port]/app/stream
    const std::string prefix = "rtmp://";
    if (url.rfind(prefix, 0) != 0 || url.size() <= prefix.size()) {
        return false;
    }

    std::string rest = url.substr(prefix.size());

    // split host[:port] and the path
    auto slash_pos = rest.find('/');
    std::string hostport = (slash_pos == std::string::npos) ? rest : rest.substr(0, slash_pos);

    if (hostport.empty()) {
        return false;
    }

    uint16_t port = 1935;
    std::string host = hostport;
    auto colon_pos = hostport.find(':');
    if (colon_pos != std::string::npos) {
        host = hostport.substr(0, colon_pos);
        auto port_str = hostport.substr(colon_pos + 1);
        if (!port_str.empty()) {
            port = static_cast<uint16_t>(std::atoi(port_str.c_str()));
        }
    }

    if (host.empty()) {
        return false;
    }

    addr_.SetPort(port);
    addr_.SetAddr(host);
    return true;
}