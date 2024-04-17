#ifndef PACKETGENERATOR_HPP
#define PACKETGENERATOR_HPP

#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"
#include "Client.h"
#include "../utils/TUI.h"

#include <vector>
#include <string>

class PacketGenerator {
    private:
        Client* client;

    public:
	    PacketGenerator(Client* client);
        vector<Message> generatePackets(std::string input);
        vector<Message> generatePackets(std::string input, int destAddr);

        vector<Message> generateAckPacket(int seqNum, int destAddr);
        vector<Message> generateMultiPacket(std::string input);
        vector<Message> generateSingleDataPacket(std::string input);
        vector<Message> generateMultiPacket(std::string input, int destAddr);
        vector<Message> generateSingleDataPacket(std::string input, int destAddr);
};

#endif //PACKETGENERATOR_HPP