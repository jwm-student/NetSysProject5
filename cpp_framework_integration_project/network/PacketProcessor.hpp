#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"
#include "PacketGenerator.hpp"
#include "CollisionAvoidance.h"
#include <vector>
//#include "DVR.h"

#ifndef PACKET_PROCESSOR_H
#define PACKET_PROCESSOR_H

class PacketProcessor {
    private:
        PacketGenerator *PG;
        CollisionAvoidance *CA;
        Client *client;
        //DVR *DVR;
        vector<char> buffer;
        int sendingSrc;
        vector<vector<int>>* routingTable;

    public:
        PacketProcessor(PacketGenerator *PG, CollisionAvoidance *CA, Client *client, vector<vector<int>>* routingTable);

        void processDataPacket(Message);
        void processAckPacket(Message);
};

#endif // :ACKET_PROCESSOR_H