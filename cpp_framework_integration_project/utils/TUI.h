#ifndef TUI_H
#define TUI_H

#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"
#include "../network/Client.h"
//#include "../network/PacketGenerator.hpp"
#include "../network/CollisionAvoidance.h"
class PacketGenerator;

class TUI {
    private:
        Client *client;
        PacketGenerator *packetGenerator;
        CollisionAvoidance *collisionAvoidance;

        char my_addr;

    public:
        TUI(Client *client, PacketGenerator *packetGenerator, CollisionAvoidance *collisionAvoidance);
        int setDestinationAddress();
        void printMenu();
        void printReachableNodes();
        void processInput(std::string input);
        int setMyAddress();
};

#endif //TUI_H