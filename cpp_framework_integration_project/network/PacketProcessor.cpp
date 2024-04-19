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
    this->buffer = {};
    cout << "packetprocessor initiated, buffer.size() = " << buffer.size() << endl;
}

void PacketProcessor::processDataPacket(Message incomingMessage){
    int destAddress = (incomingMessage.data[0] & 0b00110000) >> 4;
    int srcAddress = (incomingMessage.data[0] & 0b11000000) >> 6;
    cout << "processing data, dest addres = " << destAddress << ", srcaddress = " << srcAddress << ", Buffer size" << buffer.size() << endl;
    if(buffer.size()== 0){
        if(destAddress == client->getMyAddr()){
            if((incomingMessage.data[1] & 0b01000000) == 0b01000000){ // if the message is part of longer message
                cout << "message is longer than 1 packet!" << endl;
                sendingSrc = srcAddress;
                int receivedSeqNum = (incomingMessage.data[1] & 0b00000111);
                client->setExpSeqNum(receivedSeqNum);
                incomingMessage.data.erase(incomingMessage.data.begin(), incomingMessage.data.begin()+2);
                // Add data to buffer
                buffer.insert(buffer.end(),incomingMessage.data.begin(),incomingMessage.data.end());
                client->increaseExpSeqNum();
                vector<Message> ack = PG->generateAckPacket(receivedSeqNum, srcAddress);
                cout << "sending ACK!" << endl;
                CA->sendMessageCA(ack); // send ack
            }
            else{ // if it is a single message
                int receivedSeqNum = (incomingMessage.data[1] & 0b00000111);
                incomingMessage.data.erase(incomingMessage.data.begin(), incomingMessage.data.begin()+2);
                // Add data to buffer
                buffer.insert(buffer.end(),incomingMessage.data.begin(),incomingMessage.data.end());
                client->increaseExpSeqNum();
                vector<Message> ack = PG->generateAckPacket(receivedSeqNum, srcAddress);
                
                // Printing out received data
                cout << "Received a message from: " << srcAddress << ":\n" << endl;
                for (char c : buffer) {
				    std::cout << c;
			    }
                cout << "\n" << endl;
                buffer.clear();
                CA->setReceivedMessageType(FREE); //TO-DO: Solve less hacky
                CA->sendMessageCA(ack); // send ack
            }
        } 
        // else if(){ //else if nextHop is me, perform this code and send the message to the new dest

        // }
        
        else {
            //received Message, but not for me.
            printf("received message! But not for me ");
        }

    } else if(buffer.size() != 0){
        cout << "received another message, buffer is non-zero!" << endl;
        // If I'm the destination and the sender is the same sender as before.
        if((destAddress == client->getMyAddr()) && (srcAddress == sendingSrc)){
            int receivedSeqNum = (incomingMessage.data[1] & 0b00000111);
            if(receivedSeqNum == client->getExpSeqNum()){
                incomingMessage.data.erase(buffer.begin(), buffer.begin()+2);
                // Add data to buffer
                buffer.insert(buffer.end(),incomingMessage.data.begin(),incomingMessage.data.end());
                client->increaseExpSeqNum();
                vector<Message> ack = PG->generateAckPacket(receivedSeqNum, srcAddress);
                CA->sendMessageCA(ack); // send ack
            }
            else{
                vector<Message> ack = PG->generateAckPacket(receivedSeqNum, srcAddress);
                CA->sendMessageCA(ack); // send ack
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