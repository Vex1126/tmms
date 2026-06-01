#include"network/base/SocketOpt.h"
#include"network/net/InetAddress.h"
#include"network/net/EventLoop.h"
#include"network/net/EventLoopThread.h"
#include"network/net/EventLoopThreadPool.h"
#include"network/net/Connection.h"
#include"network/net/TcpConnection.h"
#include<iostream>
#include"network/TcpServer.h"
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
        TcpServer server(loop,addr);
        server.SetMessageCallBack([](const TcpConnectionPtr&con,MsgBuffer&msg){
            std::cout << "host:" << con->GetPeerAddr().ToIpPort() <<" msg:"<<msg.peek();
            msg.retrieveAll();
            con->Send(response,strlen(response));
        });

        server.SetNewConnectionCallBack([](const TcpConnectionPtr&con){
            con->SetWriteCompleteCallBack([](const TcpConnectionPtr&con){
            std::cout << "write complete host:" << con->GetPeerAddr().ToIpPort();
            con->ForceClose();    
            });
        });
        server.Start();
        while(1){
            std::this_thread::sleep_for(std::chrono::seconds(1));              
        }         
    }

    return 0;
}