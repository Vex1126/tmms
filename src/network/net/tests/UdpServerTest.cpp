#include"network/base/SocketOpt.h"
#include"network/net/InetAddress.h"
#include"network/net/EventLoop.h"
#include"network/net/EventLoopThread.h"
#include"network/net/EventLoopThreadPool.h"
#include"network/net/Connection.h"
#include"network/net/TcpConnection.h"
#include<iostream>
#include"network/UdpServer.h"
#include"network/net/UdpSocket.h"
using namespace tmms::network;
EventLoopThread t;
const char * response = "HTTP/1.1 200 OK\r\n"
                        "Server: tmms\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: 0\r\n"
                        "\r\n";
int main(int argc, char const *argv[])
{   
    std::vector<TcpConnectionPtr>list;
    t.Run();
    EventLoop *loop = t.Loop();
    if(loop){
        InetAddress addr("0.0.0.0:34444");
        std::shared_ptr<UdpServer> server= std::make_shared<UdpServer>(loop,addr);
        server->SetRecvMsgCallBack([&server](const InetAddress & addr,MsgBuffer& buf){
            std::cout << "host:" << addr.ToIpPort() <<" msg:"<<buf.peek() << std::endl;
            struct sockaddr_in6 addr_in6;
            addr.GetSockAddr((sockaddr*)&addr_in6);
            server->Send(buf.peek(),buf.readableBytes(),(sockaddr*)&addr_in6,sizeof(addr_in6));
            buf.retrieveAll();
        });

        server->SetWriteCompleteCallBack([](const UdpSocketPtr&con){
            std::cout << "peer:" << con->GetPeerAddr().ToIpPort();
        });
        server->Start(); 
        while(1){
            std::this_thread::sleep_for(std::chrono::seconds(1));              
        }         
    }

    return 0;
}