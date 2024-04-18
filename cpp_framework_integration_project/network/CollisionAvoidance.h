#ifndef COLLISIONAVOIDANCE_H
#define COLLISIONAVOIDANCE_H

#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"


#ifndef COLLISION_AVOIDANCE_H
#define COLLISION_AVOIDANCE_H
class CollisionAvoidance {
    private:
        MessageType typeMessage;
        BlockingQueue< Message > *senderQueue;

    public:
        CollisionAvoidance(BlockingQueue< Message > *senderQueue);
        MessageType getReceivedMessageType();
        void setReceivedMessageType(MessageType);
        bool queueIsBusy(MessageType);
        void sendMessageCA(vector<Message>);

};

#endif //COLLISIONAVOIDANCE_H