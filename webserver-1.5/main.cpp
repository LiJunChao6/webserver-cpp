#include <sys/timerfd.h>
#include <iostream>
#include <ostream>
#include <string.h>
#include <thread>


#include "EventLoopThread.h"
#include "Poller.h"
#include "TimerQueue.h"
#include "Timer.h"
#include "SocketsOps.h"
#include "TcpServer.h"

std::string message;

void onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peerAddress().toHostPort().c_str());
    conn->send(message);
  }
  else
  {
    printf("onConnection(): connection [%s] is down\n",
           conn->name().c_str());
  }
}

void onWriteComplete(const TcpConnectionPtr& conn)
{
   printf("onWriteComplete\n");
  conn->send(message);
}

void onMessage(const TcpConnectionPtr& conn,
               Buffer* buf,
               Timestamp receiveTime)
{
  printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
         buf->readableBytes(),
         conn->name().c_str(),
         receiveTime.toFormattedString().c_str());

  buf->retrieveAll();
}

int main()
{
  printf("main(): pid = %d\n", getpid());

  std::string line;
  for (int i = 33; i < 127; ++i)
  {
    line.push_back(char(i));
  }
  line += line;

  for (size_t i = 0; i < 127-33; ++i)
  {
    message += line.substr(i, 72) + '\n';
  }

  InetAddress listenAddr(9981);
  EventLoop loop;

  TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  server.setWriteCompleteCallback(onWriteComplete);
  server.start();

  loop.loop();
}