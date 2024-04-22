#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"
#include "PacketGenerator.hpp"
#include "CollisionAvoidance.h"
#include <vector>
#include "DVR.h"

#ifndef PACKET_PROCESSOR_H
#define PACKET_PROCESSOR_H

class PacketProcessor {
    private:
        PacketGenerator *PG;
        CollisionAvoidance *CA;
        Client *client;
        DVR *dvr;
        vector<vector<int>> internalRoutingTable;
        int sendingSrc;
        vector<vector<int>>* routingTable;
        vector<char> newBuffer;

    public:
        PacketProcessor(PacketGenerator *PG, CollisionAvoidance *CA, Client *client, DVR *dvr, vector<vector<int>>* routingTable);

        void processDataPacket(Message);
        void processAckPacket(Message);
};

#endif // :ACKET_PROCESSOR_H