#include"network/base/SocketOpt.h"
#include"network/net/InetAddress.h"
#include"network/net/EventLoop.h"
#include"network/net/EventLoopThread.h"
#include"network/net/EventLoopThreadPool.h"
#include"network/net/Acceptor.h"
#include"network/net/Connection.h"
#include"network/net/TcpConnection.h"
#include<iostream>
using namespace tmms::network;
EventLoopThread t;
const char* request = "GET / HTTP/1.0\r\n"
                      "Host: 127.0.0.1\r\n"
                      "Accept: */*\r\n"
                      "Content-Type: text/html\r\n"
                      "Content-Length: 0\r\n"
                      "\r\n";

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
        InetAddress sever("0.0.0.0:34444");
        std::shared_ptr<Acceptor> accptr = std::make_shared<Acceptor>(loop,sever);
        accptr->SetAcceptorCallBack([&loop,&sever,&list](int fd,const InetAddress addr){
            std::cout << "host:" << addr.ToIpPort() <<std::endl;
            TcpConnectionPtr connect = std::make_shared<TcpConnection>(loop,fd,sever,addr);
            connect->SetRecvMsgCallBack([](const TcpConnectionPtr & con,MsgBuffer & buf){
                std::cout<<"revc msg:"<< buf.peek()<<std::endl;
                buf.retrieveAll();
                con->Send(response,strlen(response));
            }); 

            connect->SetWriteCompleteCallBack([&loop](const TcpConnectionPtr & con){
                std::cout<<"write complete host:"<< con->GetPeerAddr().ToIpPort() <<std::endl;
                //loop->DelEvent(con);
                //con->ForceClose();
            });
            list.push_back(connect);
            loop->AddEvent(connect);
        });
        accptr->Start();
        while(1){
            std::this_thread::sleep_for(std::chrono::seconds(1));              
        }         
    }

    return 0;
}
