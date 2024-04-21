#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"
#include "PacketGenerator.hpp"
#include "CollisionAvoidance.h"
#include "DVR.h"

#ifndef PACKET_PROCESSOR_H
#define PACKET_PROCESSOR_H

class PacketProcessor {
    private:
        PacketGenerator *PG;
        CollisionAvoidance *CA;
        Client *client;
        DVR *dvr;
        vector<char> buffer;
        int sendingSrc;


    public:
        PacketProcessor(PacketGenerator *PG, CollisionAvoidance *CA, Client *client, DVR *dvr);

        void processDataPacket(Message);
        void processAckPacket(Message);
};

#endif // :ACKET_PROCESSOR_H