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

#include "TUI.h"
#include "../network/PacketGenerator.hpp"

using namespace std;

TUI::TUI(Client* client, PacketGenerator *packetGenerator, CollisionAvoidance *collisionAvoidance){
    this->client = client;
    this->packetGenerator = packetGenerator;
    this->collisionAvoidance = collisionAvoidance;
}

int TUI::setDestinationAddress(){
    bool input_valid = false;
    string destAddress;

    //wrongly initialized, so it has to change.
    char dest_addr = '4';
    printf("You can set the destination address to the following nodes: ");
    //print statement with possibilities, excluding their own.

    //Loop until valid input
    while(!input_valid){
        getline(cin, destAddress);
        dest_addr = destAddress.at(0);
        my_addr = client->getMyAddr();
        if(dest_addr != my_addr && (my_addr == '0' || my_addr == '1' || my_addr == '2' || my_addr == '3')){
			input_valid = true;
        } else {
            cout << "Invalid input, please enter 0, 1, 2 or 3." << destAddress <<endl;
        }
    }
    return dest_addr - '0';
}

void TUI::processInput(std::string input){
    if(input.size() > 2 || input.size() <= 1){
        cout << "Invalid command. Type '-H' to see the list of available commands" << endl;
    }
    else if (input == "-H")
    {
        printMenu();
    }
    else if (input == "-B")
    {
        string messageContent;
        cout << "What is the message would you like to send?" << endl;
        getline(cin, messageContent);
        if(messageContent.size() < (16 * 30)){
            cout << "Broadcasting your message" << endl;
            // Give input to network layer
            vector<Message> packets = packetGenerator->generatePackets(input, client);
			//Send message via Collision Avoidance
			collisionAvoidance->sendMessageCA(packets);
        }
        else{
            cout << "You're message is too long. Please use the command again and write a shorter message" << endl;
        }
    }
    else if (input == "-U")
    {
        int destAddr = setDestinationAddress();
        string messageContent;
        cout << "What is the message would you like to send?" << endl;
        getline(cin, messageContent);
        if(messageContent.size() < (16 * 30)){
            cout << "Sending your message to " << destAddr << endl;
            // TO-DO: give input to network layer
        }
        else{
            cout << "You're message is too long. Please use the command again and write a shorter message" << endl;
        }
        
    }
    else if (input == "-P")
    {
        printReachableNodes();
    }
    else if (input == "-Q")
    {
        cout << "Use 'Ctrl + C' to quit the application." << endl;
    }    
}

void TUI::printMenu(){
    cout << "Commands: \n" <<
    "-H     Help, prints command list \n" <<
    "-B     Broadcast, sends message to all nodes \n" <<
    "-U     Unicast, sends message to one node \n" <<
    "-P     Prints reachable nodes" <<
    "-Q     Quit \n" 
    << endl;
}

void TUI::printReachableNodes(){
    cout << "This function still has to be written. " << endl;
}