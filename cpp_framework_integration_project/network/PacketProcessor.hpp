#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"
#include "PacketGenerator.hpp"
#include "CollisionAvoidance.h"

#ifndef PACKET_PROCESSOR_H
#define PACKET_PROCESSOR_H

class PacketProcessor {
    private:
        PacketGenerator *PG;
        CollisionAvoidance *CA;
        Client *client;
        vector<char> buffer;
        int sendingSrc;


    public:
        PacketProcessor(PacketGenerator *PG, CollisionAvoidance *CA, Client *client);

        void processDataPacket(Message);
        void processAckPacket(Message);
};

#endif // PACKET_PROCESSOR_H