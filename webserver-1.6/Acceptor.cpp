#include "Acceptor.h"
#include "SocketsOps.h"
Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie()),
      acceptChannel_(loop, acceptSocket_.fd()),
      listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
} 

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddress peerAddr(0);
    std::cout<<"Acceptor::handleRead()"<<std::endl;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            std::cout<<"Acceptor::newConnectionCallback_()"<<std::endl;
            newConnectionCallback_(connfd, peerAddr);
        } else {
            sockets::close(connfd);
        }
    }
}