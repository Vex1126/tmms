#include"network/base/SocketOpt.h"
#include"network/net/InetAddress.h"
#include"network/net/EventLoop.h"
#include"network/net/EventLoopThread.h"
#include"network/net/EventLoopThreadPool.h"
#include"network/net/Connection.h"
#include"network/net/TcpConnection.h"
#include"mmedia/rtmp/RtmpServer.h"
#include<iostream>
#include"network/TcpServer.h"

using namespace tmms::network;
using namespace tmms::mm;
EventLoopThread t;
int main(int argc, char const *argv[])
{   
    t.Run();
    EventLoop *loop = t.Loop();
    if(loop){
        InetAddress addr("0.0.0.0:1935");
        std::shared_ptr<RtmpServer> server= std::make_shared<RtmpServer>(loop,addr);
        server->Start();
        while(1){
            std::this_thread::sleep_for(std::chrono::seconds(1));              
        }         
    }

    return 0;
}