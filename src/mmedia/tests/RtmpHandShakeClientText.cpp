#include"network/base/SocketOpt.h"
#include"network/net/InetAddress.h"
#include"network/net/EventLoop.h"
#include"network/net/EventLoopThread.h"
#include"network/net/EventLoopThreadPool.h"
#include"network/net/Acceptor.h"
#include"network/net/Connection.h"
#include"network/net/TcpConnection.h"
#include"mmedia/rtmp/RtmpHandShake.h"
#include<iostream>
#include"mmedia/rtmp/RtmpClient.h"
using namespace tmms::network;
using namespace tmms::mm;
using RtmpHandShakePtr = std::shared_ptr<RtmpHandShake>;
EventLoopThread t;

class RtmpHandelIMP:public RtmpHandler{
    public:
            RtmpHandelIMP(){}
            ~RtmpHandelIMP(){}
            void OnNewConnection(const TcpConnectionPtr &conn)override{

            }
            void OnConnectionDestroy(const TcpConnectionPtr &conn)override{

            }
            void OnRecv(const TcpConnectionPtr &conn ,const PacketPtr &data)override{
                std::cout << "recv type:" << data->PacketType() <<" size:" << data->PacketSize()<<std::endl;
            }
            void OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data)override{
                std::cout << "recv type:" << data->PacketType() <<" size:" << data->PacketSize()<<std::endl;
            }
            void OnActive(const ConnectionPtr &conn)override{

            }

            virtual bool OnPlay(const TcpConnectionPtr &con,const std::string &session_mame,const std::string &parma){return false;}
            virtual bool OnPublish(const TcpConnectionPtr &con,const std::string &session_mame,const std::string &parma){return false;}
            virtual void OnPause(const TcpConnectionPtr &con,bool Pause){}
            virtual void OnSeek(const TcpConnectionPtr &con,bool time){}
};

int main(int argc, char const *argv[])
{   
    std::vector<TcpConnectionPtr>list;
    t.Run();
    EventLoop *loop = t.Loop();
    if(loop){
        RtmpClient client(loop,new RtmpHandelIMP());
        client.Play("rtmp://127.0.0.1:1935/live/stream");
        while(1){
            std::this_thread::sleep_for(std::chrono::seconds(1));              
        }         
    }

    return 0;
}
