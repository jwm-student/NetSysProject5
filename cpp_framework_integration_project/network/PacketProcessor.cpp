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

PacketProcessor::PacketProcessor(PacketGenerator* PG, CollisionAvoidance* CA, Client *client, DVR *dvr, vector<vector<int>>* routingTable){
    this->PG = PG;
    this->CA = CA;
    this->client = client;
    this->dvr = dvr;
    this->routingTable = routingTable;
}

void PacketProcessor::processDataPacket(Message incomingMessage){
    int destAddress = (incomingMessage.data[0] & 0b00110000) >> 4;
    int srcAddress = (incomingMessage.data[0] & 0b11000000) >> 6;
    int nextHop = (incomingMessage.data[1] & 0b00011000) >> 3;
    // cout << "processing data, dest addres = " << destAddress << ", srcaddress = " << srcAddress << ", Buffer size" << buffer.size() << endl;
//    DVR.resetTimer(srcAddress);

    if(client->getBuffer().size() == 0 && (destAddress == client->getMyAddr())){
            if((incomingMessage.data[1] & 0b01000000) == 0b01000000){ // if the message is part of longer message
                cout << "message is longer than 1 packet!" << endl;
                sendingSrc = srcAddress;
                int receivedSeqNum = (incomingMessage.data[1] & 0b00000111);
                client->setExpSeqNum(receivedSeqNum);
                incomingMessage.data.erase(incomingMessage.data.begin(), incomingMessage.data.begin()+2);
                // Add data to buffer
                newBuffer.insert(newBuffer.end(),incomingMessage.data.begin(),incomingMessage.data.end());
                client->setBuffer(newBuffer);

                client->increaseExpSeqNum();
                vector<Message> ack = PG->generateAckPacket(receivedSeqNum, srcAddress);

                //print the new buffer:
                client->printBuffer();

                // clear newBuffer
                newBuffer.clear();

                cout << "sending ACK!" << endl;
                CA->sendMessageCA(ack); // send ack
            }
            else{ // if it is a single message
                int receivedSeqNum = (incomingMessage.data[1] & 0b00000111);
                incomingMessage.data.erase(incomingMessage.data.begin(), incomingMessage.data.begin()+2);
                // Add data to buffer
                newBuffer.insert(newBuffer.end(),incomingMessage.data.begin(),incomingMessage.data.end());
                client->setBuffer(newBuffer);


                client->increaseExpSeqNum();
                vector<Message> ack = PG->generateAckPacket(receivedSeqNum, srcAddress);
                
                // Printing out received data
                // cout << "Received a message from: " << srcAddress << ":\n" << endl;
                // for (char c : newBuffer) {
				//     std::cout << c;
			    // }
                client->printBuffer();
                cout << "\n" << endl;

                //clear newBuffer and buffer after receiving single message
                newBuffer.clear();
                // client->clearBuffer();

                CA->setReceivedMessageType(FREE); //TO-DO: Solve less hacky
                CA->sendMessageCA(ack); // send ack
            }

    } else if(client->getBuffer().size() != 0 && (destAddress == client->getMyAddr())){
        cout << "received another message, buffer is non-zero!" << endl;
        // If I'm the destination and the sender is the same sender as before.
        if((destAddress == client->getMyAddr()) && (srcAddress == sendingSrc)){
            int receivedSeqNum = (incomingMessage.data[1] & 0b00000111);
            if(receivedSeqNum == client->getExpSeqNum()){
                incomingMessage.data.erase(newBuffer.begin(), newBuffer.begin()+2);
                // Add data to buffer
                newBuffer.insert(newBuffer.end(),incomingMessage.data.begin(),incomingMessage.data.end());
                client->setBuffer(newBuffer);

                client->increaseExpSeqNum();
                vector<Message> ack = PG->generateAckPacket(receivedSeqNum, srcAddress);
                if((incomingMessage.data[1] & 0b01000000) == 0){
                    cout << "Received a message from: " << srcAddress << ":\n" << endl;
                    // for (char c : buffer) {
                        // std::cout << c;
                    // }
                }

                //clear newBuffer after usage
                newBuffer.clear();

                client->printBuffer();
                CA->sendMessageCA(ack); // send ack
            }
            else{ //if last packet of a sequence
                vector<Message> ack = PG->generateAckPacket(receivedSeqNum, srcAddress);
                CA->sendMessageCA(ack); // send ack
                incomingMessage.data.erase(newBuffer.begin(), newBuffer.begin()+2);
                newBuffer.insert(newBuffer.end(),incomingMessage.data.begin(),incomingMessage.data.end());
                client->setBuffer(newBuffer);
                client->printBuffer();

                client->clearBuffer();
                newBuffer.clear();
            }
        } 
    }
    // If I'm not the destination, check if I'm the nextHop.
    else if (nextHop == client->getMyAddr())
    {
        printf("I am the next hop, going to forward!");
        // New next hop is found, send same packet with new nextHop
        vector<Message> forwardMessage = PG->generateNextHopPacketVec(incomingMessage,destAddress);
        CA->sendMessageCA(forwardMessage);
    }
    else{
        printf("Received a packet but I got nothing to do with it!");
    }
}

void PacketProcessor::processAckPacket(Message message){
    int recSeqNum = message.data[1] & 0b00000111;
    if(recSeqNum == client->getExpSeqNum()){
        client->receivedACK = TRUE;
        cout << "receivedACK is set to true" << endl;
    }
    else{
        client->receivedACK = FALSE;
    }
}