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

TUI::TUI(Client* client, PacketGenerator *packetGenerator, CollisionAvoidance *collisionAvoidance, vector<vector<int>>* routingTable){
    this->client = client;
    this->packetGenerator = packetGenerator;
    this->collisionAvoidance = collisionAvoidance;
    this->routingTable = routingTable;
}

int TUI::setDestinationAddress(){
	bool input_valid = false;
	string addrInput;
	int dest_addr = -1; // initialized to wrong value so it has to be changed.

    cout << "Please enter the address of the node you want to send a message to" << endl;
	// Loop until valid input
	while(!input_valid){
		getline(cin,addrInput);
		if(addrInput == "0" || addrInput == "1" || addrInput == "2" || addrInput == "3"){
			input_valid = true;
		}
		else{
			cout << "Invalid input, please enter 0, 1, 2 or 3." << addrInput <<endl;
		}
	}
    dest_addr = int(addrInput.at(0) - '0');
    return dest_addr;
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
            vector<Message> packets = packetGenerator->generatePackets(messageContent);
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
            // Give input to network layer
            vector<Message> packets = packetGenerator->generatePackets(messageContent,destAddr);
            
			//Send message via Collision Avoidance
			collisionAvoidance->sendMessageCA(packets);
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
    // Check if the pointer is not null
    if (routingTable) {
        // Dereference the pointer to access the vector
        const std::vector<std::vector<int>>& vec = *routingTable;

        std::cout << "Nodes that are reachable: \n" << std::endl;

        // Loop through the rows
        for(size_t i = 0; i < vec.size(); i++) {
            // Loop through the columns
            for(size_t j = 0; j < vec[i].size(); j++) {
                // Access the element using at() function to perform bounds checking
                try {
                    if(vec.at(i).at(j) < 99) {
                        std::cout << i << std::endl;
                        break; 
                    }
                } catch(const std::out_of_range& e) {
                    std::cerr << "Error: Out of range access at (" << i << ", " << j << ")" << std::endl;
                    return;
                }
            }
        }
    }
    else {
        std::cerr << "Error: null pointer to routing table" << std::endl;
    }
}


//TO-DO Implement choosing own address function