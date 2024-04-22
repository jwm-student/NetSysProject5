#include <iostream>
#include <string>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#elif __linux__
#include <sys/socket.h>    	//socket
#include <arpa/inet.h> 		//inet_addr
#endif

#include "CollisionAvoidance.h"

using namespace std;
CollisionAvoidance::CollisionAvoidance(BlockingQueue< Message > *senderQueue, Client* client){
    this->senderQueue = senderQueue;
    this->client = client;
}

MessageType CollisionAvoidance::getReceivedMessageType(){
    return typeMessage;
}

void CollisionAvoidance::setReceivedMessageType(MessageType newMessageType){
    typeMessage = newMessageType;
    // printf("ik ben lekker aan het setten");
}
bool CollisionAvoidance::queueIsBusy(MessageType RM){
    while(getReceivedMessageType() == BUSY || getReceivedMessageType() == SENDING || getReceivedMessageType() == DATA || getReceivedMessageType() == DATA_SHORT){
        //printf("Ik ben aan het loopen");
        int rn = (rand() % 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(rn));
    }
    return false;
}

void CollisionAvoidance::sendMessageCA(vector<Message> packets){
    while(packets.size()>0){
       
        //pop the same message out of senderMessageVector.
	
        //Je mag deze wel erasen, maar zorg ervoor dat hij eerst in een 
        //andere vector<Message> opgeslagen wordt. Zodat pas bij een ACK hij daadwerkelijk loesoe is.
        //EN tot die tijd evt. opnieuw gestuurd kan worden bij geen ACK.
        Message sendThisMessage = packets.front();
        packets.erase(packets.begin());
        
        int rn = (rand() % 200);
        std::this_thread::sleep_for(std::chrono::milliseconds(rn));
        if(queueIsBusy(getReceivedMessageType()) == false){
            senderQueue->push(sendThisMessage);
        }
    }
}