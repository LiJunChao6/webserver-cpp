#include <iostream>
#include "Poller.h"
#include "Channel.h"
#include <time.h>
#include <poll.h>
#include <assert.h>
Poller::Poller(EventLoop *loop) : ownerLoop_(loop){}

Poller::~Poller(){}

// 调用poll(2)获得当前活动的IO事件，然后将事件填充调用方传入的activeChannels
Timestamp Poller::poll(int timeoutMs, ChannelList *activeChannels) {
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);

    // Timestamp now(TimeStamp::now());

    if(numEvents > 0) {
        std::cout << numEvents << " events happended" <<std::endl;
        //遍历pollfds_，找出有活动事件的fd，把它对应的Channel填入activeChannels
        fillActiveChannels(numEvents, activeChannels);
    }
    else if (numEvents == 0) {
        std::cout << " nothing happended" <<std::endl;
    }
    else {
        std::cout << "Poller::poll()";
    }

    return 1;
}

// 当前活动事件revents会保存在activeChannel中，供Channel::handleEvent()使用
void Poller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const {
    for(PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; pfd++) {
        if(pfd->revents > 0) {
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel* channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->setRevents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}

// 负责维护和更新pollfds_数组
void Poller::updateChannel(Channel *channel) {
    assertInLoopThread();
    std::cout << "fd = " << channel->fd() << " events = " << channel->events() <<std::endl;
    if(channel->index() < 0) {
        // 新Channel 添加到pollfd数组和map中
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size() - 1);
        channel->setIndex(idx);
        channels_[pfd.fd] = channel;
    }
    else {
        // 更新已有的Channel
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent()) 
        {
            // igonre this polfd
            pfd.fd = -1;    // 1
        }
    }
}