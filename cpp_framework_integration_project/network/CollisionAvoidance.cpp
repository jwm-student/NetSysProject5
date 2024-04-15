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

Message CollisionAvoidance::getReceivedMessageType(){
	return typeMessage;
}
void CollisionAvoidance::setReceivedMessageType(Message newMessageType){
	typeMessage = newMessageType;
	// printf("ik ben lekker aan het setten");
}
bool CollisionAvoidance::queueIsBusy(MessageType RM){
    while(getReceivedMessageType().type == BUSY || getReceivedMessageType().type == SENDING || getReceivedMessageType().type == DATA){
        int rn = (rand() % 50);
        std::this_thread::sleep_for(std::chrono::milliseconds(rn));
    }
    return false;
}