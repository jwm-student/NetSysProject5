#include <iostream>
#include <string>
#include <cstring>
#include <bitset>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#elif __linux__
#include <sys/socket.h>    	//socket
#include <arpa/inet.h> 		//inet_addr
#endif

#include "PacketProcessor.hpp"

PacketProcessor::PacketProcessor(PacketGenerator* PG, CollisionAvoidance* CA, Client *client){
    this->PG = PG;
    this->CA = CA;
    this->client = client;
}

void PacketProcessor::processDataPacket(Message incomingMessage){
    int destAddress = (incomingMessage.data[0] & 0b00110000) >> 4;
    int srcAddress = (incomingMessage.data[0] & 0b11000000) >> 6;
    if(buffer.size()== 0){
        if(destAddress == (client->getMyAddr() - '0')){
            if((incomingMessage.data[1] & 0b01000000) == 0b01000000){
                sendingSrc = srcAddress;
                int receivedSeqNum = (incomingMessage.data[1] & 0b00000111);
                client->setExpSeqNum(receivedSeqNum);
                incomingMessage.data.erase(buffer.begin(), buffer.begin()+2);
                // Add data to buffer
                buffer.insert(buffer.end(),incomingMessage.data.begin(),incomingMessage.data.end());
                client->increaseExpSeqNum();
                vector<Message> ack = PG->generateAckPacket(receivedSeqNum, this->client, srcAddress);
                CA->sendMessageCA(ack,client->getSenderQueue()); // send ack
            }
        } 
        // else if(){ //else if nextHop is me, perform this code and send the message to the new dest

        // }
        
        else {
            //received Message, but not for me.
            printf("received message! But not for me ");
        }

    } else if(buffer.size() != 0){
        // If I'm the destination and the sender is the same sender as before.
        if(destAddress == (client->getMyAddr() - '0') && srcAddress == sendingSrc){
            int receivedSeqNum = (incomingMessage.data[1] & 0b00000111);
            if(receivedSeqNum == client->getExpSeqNum()){
                incomingMessage.data.erase(buffer.begin(), buffer.begin()+2);
                // Add data to buffer
                buffer.insert(buffer.end(),incomingMessage.data.begin(),incomingMessage.data.end());
                client->increaseExpSeqNum();
                vector<Message> ack = PG->generateAckPacket(receivedSeqNum, this->client, srcAddress);
                CA->sendMessageCA(ack,client->getSenderQueue()); // send ack
            }
            else{
                vector<Message> ack = PG->generateAckPacket(receivedSeqNum, this->client, srcAddress);
                CA->sendMessageCA(ack,client->getSenderQueue()); // send ack
            }
        }
    }
}
void PacketProcessor::processAckPacket(Message incomingAckMessage){
    //Er vanuitgaand dat seqnum opzelfde plek zit als bij DATA.
    int receivedSeqNum = (incomingAckMessage.data[1] & 0b00000111);

    if(receivedSeqNum == client->getExpSeqNum()){
        
    }
}