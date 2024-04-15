#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"
#include "Client.h"

#include <vector>
#include <string>

class PacketGenerator {
    private:
        enum PacketType{
            ACK,
            MULTI,
            SINGLE
        };

    public:
	    PacketGenerator();
        vector<Message> generatePackets(std::string input, Client* client);
        vector<Message> generateAckPacket(int seqNum, Client* client, int destAddr);
        vector<Message> generateMultiPacket(std::string input, Client* client);
        vector<Message> generateSingleDataPacket(std::string input, Client* client);
};