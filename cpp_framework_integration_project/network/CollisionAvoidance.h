#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"

class CollisionAvoidance {
    private:
        Message typeMessage;

    public:
	    Message getReceivedMessageType();
	    void setReceivedMessageType(Message);
        bool queueIsBusy(MessageType);
};