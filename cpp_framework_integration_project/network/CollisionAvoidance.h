#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"

class CollisionAvoidance {
    private:
        MessageType typeMessage;

    public:
	    MessageType getReceivedMessageType();
	    void setReceivedMessageType(MessageType);
        bool queueIsBusy(MessageType);
        void sendMessageCA(vector<Message>, BlockingQueue< Message >* senderQueue);

};