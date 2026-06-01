#include"network/base/SocketOpt.h"
#include"network/net/InetAddress.h"
#include"network/net/EventLoop.h"
#include"network/net/EventLoopThread.h"
#include"network/net/EventLoopThreadPool.h"
#include"network/net/Acceptor.h"
#include"network/net/Connection.h"
#include"network/net/TcpConnection.h"
#include<iostream>
#include"network/TcpClient.h"
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
                        "Connection: close\r\n"
                        "\r\n";
int main(int argc, char const *argv[])
{   
    std::vector<TcpConnectionPtr>list;
    t.Run();
    EventLoop *loop = t.Loop();
    if(loop){
        InetAddress sever("127.0.0.1:34444");
        std::shared_ptr<TcpClient> client =std::make_shared<TcpClient>(loop,sever);
            client->SetRecvMsgCallBack([](const TcpConnectionPtr & con,MsgBuffer & buf){
                std::cout<<"revc msg:"<< buf.peek()<<std::endl;
                buf.retrieveAll();
            }); 

            client->SetWriteCompleteCallBack([&loop](const TcpConnectionPtr & con){
                std::cout<<"write complete host:"<< con->GetPeerAddr().ToIpPort() <<std::endl;
            });

            client->SetCloseCallBack([](const TcpConnectionPtr & con){
                if(con){
                 std::cout<<"server:"<< con->GetPeerAddr().ToIpPort() <<"close"<<std::endl;   
                }
            });

            client->SetConnectCallBack([](const TcpConnectionPtr & con,bool conn){
                if(conn){
                    con->Send(request,strlen(request));
                }
            });
            client->Connect();
        while(1){
            std::this_thread::sleep_for(std::chrono::seconds(1));              
        }         
    }

    return 0;
}
