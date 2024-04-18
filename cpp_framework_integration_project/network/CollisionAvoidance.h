#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"


#ifndef COLLISION_AVOIDANCE_H
#define COLLISION_AVOIDANCE_H
class CollisionAvoidance {
    private:
        MessageType typeMessage;

    public:
        MessageType getReceivedMessageType();
        void setReceivedMessageType(MessageType);
        bool queueIsBusy(MessageType);
        void sendMessageCA(vector<Message>, BlockingQueue< Message >* senderQueue);
};

#endif // COLLISION_AVOIDANCE_H