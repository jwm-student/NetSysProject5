#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"

#ifndef COLLISIONAVOIDANCE_H
#define COLLISIONAVOIDANCE_H

class CollisionAvoidance {
    private:
        MessageType typeMessage;

    public:
        MessageType getReceivedMessageType();
        void setReceivedMessageType(MessageType);
        bool queueIsBusy(MessageType);
        void sendMessageCA(vector<Message>, BlockingQueue< Message >* senderQueue);
};
#endif