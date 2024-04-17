#include "../utils/BlockingQueue.h"
#include "../utils/Message.h"
#include "../utils/MessageType.h"
#include "../network/Client.h"

class TUI {
    private:
        Client *client;
        char my_addr;

    public:
        TUI(Client *client);
        int setDestinationAddress();
};