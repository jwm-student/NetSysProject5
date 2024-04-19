#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"
#include "Client.h"
#include "CollisionAvoidance.h"
#include "PacketGenerator.hpp"

#include <string>
#include <cstring>
#include <vector>
#include <bitset>
#include <chrono>

class DVR {
    private:
        BlockingQueue< Message >*senderQueue;
        Client* client;
        PacketGenerator* packetGenerator;
        CollisionAvoidance* AC;
        vector<chrono::steady_clock::time_point> TTLvec;
    public:
        DVR(BlockingQueue< Message >*senderQueue, Client* client, PacketGenerator* packetGenerator, CollisionAvoidance* AC);
        void sendPing();
        bool routingMessageHandler(Message temp, vector<vector<int>>& routingTable);
        void sendUpdatedTable(vector<vector<int>>& routingTable, bool& sendRoutingTable);
        void timerChecker(vector<vector<int>>& routingTable, bool& sendRoutingTable);
        void resetTimer(int incomingSourceAddress);
};