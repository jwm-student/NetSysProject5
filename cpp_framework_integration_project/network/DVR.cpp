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

#include "DVR.h"

DVR::DVR(BlockingQueue< Message >*senderQueue, Client* client, PacketGenerator* packetGenerator, CollisionAvoidance* AC){
    this->senderQueue = senderQueue;
    this->client = client;
    this->packetGenerator = packetGenerator;
    this->AC = AC;
}

void DVR::sendPing(){
    Message sendMessage;
    vector<Message> pingVector = packetGenerator->generatePingPacket(client);
    AC->sendMessageCA(pingVector, senderQueue);
}

bool DVR::routingMessageHandler(Message temp, vector<vector<int>>& routingTable){
    // Input: routing message, local routing table
	// Takes a message that has been flagged as "routing" and processes it
	// It checks if the local routingtable has to be updated, and calls the 
	// sendUpdatedTable function if true.
	vector<char> data = temp.data;
	vector<unsigned int> bytes;
	vector<unsigned int> receivedTableData;
	vector<vector<unsigned int>> receivedTable;
	unsigned int receivedSourceAddress;
	int dataLength = 0;
	int tableDests = 0;
	int tableVias = 0;
	bool updatedTable = false;

	// Make sure that vectors are empty
	bytes.clear();
	receivedTableData.clear();
	receivedTable.clear();
   	receivedTable.resize(0); 

	switch(temp.type) {
		case DATA_SHORT:	//This means that it is a discovery message
		// Add neighbour to own tablex
		for (char c : data) {
			bytes.push_back(static_cast<unsigned int>(c));
		}
		receivedSourceAddress = (bytes[0] & 0b11000000)>>6;
		std::cout << "The received source address is: " << receivedSourceAddress << std::endl;
		if (routingTable[receivedSourceAddress][receivedSourceAddress] != 1){
			routingTable[receivedSourceAddress][receivedSourceAddress] = 1;
			updatedTable = true;
			std::cout << "Updated the routing table to " << routingTable[receivedSourceAddress][receivedSourceAddress] << std::endl;
		}
		break;
		case DATA:			// This means that it is a topology message
		// Import and process incoming table
		std::cout << "Received a DATA packet!" << std::endl;
		for (char c : data) {
			if (c != '\0'){
				dataLength++;	// Finds the amount of data in the packet
			}
			bytes.push_back(static_cast<unsigned int>(c));	
		}

		std::cout << "Received " << dataLength << " bytes of data" << std::endl;
		receivedSourceAddress = (bytes[0] & 0b11000000)>>6;
		std::cout << "Source address is: " << receivedSourceAddress << std::endl;

		// Added direct neighbour if not seen before
		if (routingTable[receivedSourceAddress][receivedSourceAddress] != 1){
			routingTable[receivedSourceAddress][receivedSourceAddress] = 1;
			updatedTable = true;
		}

		// Find table dimensions
		tableDests = bytes[2];
		tableVias = bytes[3];
		std::cout << "dests are: " << tableDests << " and vias are: " << tableVias << std::endl;
		
		// Remove header and dimension info from data
		for (int i = 4; i < dataLength; i++){
			receivedTableData.push_back(bytes[i]);
		}
		std::cout << "removed header" << std::endl;
		// Sort data into 2D table TODO: CHECK IF ORDER IS CORRECT
		for(int i = 0; i < tableDests; i++){
    		receivedTable.push_back(vector<unsigned int>());
    		for(int j = 0; j < tableVias; j++){    
    			receivedTable[i].push_back(receivedTableData[i * tableDests + j]);    
    		}
		}
		std::cout << "Received the following table: " << std::endl;
		for (const auto& row : receivedTable) {
        	for (const auto& elem : row) {
            	std::cout << elem << ' ';
        	}
        	std::cout << std::endl; // Newline for each row
    	}

		// for (int i = 0; i < tableDests; i++){
		// 	for (int j = 0; j < tableVias; j++){
		// 		std::cout << receivedTableData[i * tableVias + j] << std::endl;
		// 		receivedTable[i][j] = static_cast<uint8_t>(receivedTableData[i * tableVias + j]);
		// 	}
		// }

		// For a given destination, loop through all entries in the received table,
		// add 1 and see if it's lower than the current cost of destination over the
		// the source address's hop.
		for (int i = 0; i < tableDests; i++){
			for (int j = 0; j < tableVias; j++){
				if ((receivedTable[i][j] + 1) < routingTable[i][receivedSourceAddress]){
					routingTable[i][receivedSourceAddress] = (receivedTable[i][j] + 1);
					updatedTable = true;
				}
			}
		}
	}
	// Print the current table for evaluation
	for (const auto& row : routingTable) {
        for (const auto& elem : row) {
            std::cout << elem << ' ';
        }
        std::cout << '\n';
    }
	return updatedTable;
}

void DVR::sendUpdatedTable(vector<vector<int>>& routingTable, bool& sendRoutingTable){
	// This function can be called when a local routingtable has been updated
	// It sends the table to all neighbours, so that they can update theirs
	// It counts the width and height of the table, adds this to the first two bytes of data
	// Then it adds the table contents after that and sends the message over to the header constructor
	while (true){
		if (sendRoutingTable == true){
			vector<char> sendingTable;

			std::cout << "I want to send the following table: " << std::endl;
			for (const auto& row : routingTable) {
				for (const auto& elem : row) {
					std::cout << elem << ' ';
				}
				std::cout << std::endl; // Newline for each row
			}

			// Add information about table size
			std::cout << "In the sendUpdatedTable function" << std::endl;
			sendingTable.push_back(static_cast<char>(routingTable.size()));
			sendingTable.push_back(static_cast<char>(routingTable[0].size()));
			std::cout << "Got the table size data" << std::endl;
			std::cout << "Sendingtable now has these two values: " << sendingTable[0] << " , " << sendingTable[1] << std::endl;
			
			// Add table data
			for (unsigned int i = 0; i < routingTable.size(); i++){
				for (unsigned int j = 0; j < routingTable[0].size(); j++){
					std::cout << "i is: " << i << " , j is: " << j << std::endl;
					sendingTable.push_back(static_cast<char>(routingTable[i][j]));
					std::bitset<8> bits(static_cast<char>(routingTable[i][j]));
					std::string bitString = bits.to_string();
					std::cout << "added char of value " << bitString << std::endl;
				}
			}

			std::cout << "size of the sendingtable is: " << sendingTable.size() << std::endl;
			vector<Message> routingVector = packetGenerator->generateRoutingPacket(sendingTable,client);
			Message sendMessage;
			sendMessage = routingVector[0];
			AC->sendMessageCA(routingVector, senderQueue);
			std::cout << std::endl << "Table sent!" << std::endl;
			sendRoutingTable = false;
		}

	}

}