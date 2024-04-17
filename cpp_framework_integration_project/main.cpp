#include <thread>
#include <string>
#include <cstring>
#include <vector>
#include <bitset>
#include <chrono>
#include <functional>

#include "utils/BlockingQueue.h"
#include "network/Client.h"
#include "utils/Message.h"
#include "network/CollisionAvoidance.h"
#include "network/PacketGenerator.hpp"

/**
* This is just some example code to show you how to interact 
* with the server using the provided client and two queues.
* Feel free to modify this code in any way you like!
*/

// The address to connect to. Set this to localhost to use the audio interface tool.
std::string SERVER_ADDR = "netsys.ewi.utwente.nl"; //"127.0.0.1"
// The port to connect to. 8954 for the simulation server
int SERVER_PORT = 8954;
// The frequency to connect on.
int FREQUENCY = 8000;//TODO: Set this to your group frequency!
// The token you received for your frequency range
std::string TOKEN = "cpp-05-AYKI3U9SX758O0EPJT";


using namespace std;

void readInput(BlockingQueue< Message >*senderQueue, char addr, CollisionAvoidance* AC, Client* client, PacketGenerator* packetGenerator) {
while (true) {
		string input;
		std::cout << "Enter your message: " << std::endl;
		getline(cin, input); //read input from stdin
		if(input.size() < 16*30){
			vector<Message> packets = packetGenerator->generatePackets(input, client);
			//Send message using CA
			AC->sendMessageCA(packets, senderQueue);
		}
		else{
			std::cout << "Message too long, please write a shorter message!" << std::endl;
		}
	}
}
void sendUpdatedTable(vector<vector<int>>& routingTable, PacketGenerator* PacketGenerator, Client* client,CollisionAvoidance* AC,BlockingQueue< Message >*senderQueue, bool& sendRoutingTable){
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
			vector<Message> routingVector = PacketGenerator->generateRoutingPacket(sendingTable,client);
			Message sendMessage;
			sendMessage = routingVector[0];
			AC->sendMessageCA(routingVector, senderQueue);
			std::cout << std::endl << "Table sent!" << std::endl;
			sendRoutingTable = false;
		}

	}

}

bool routingMessageHandler(Message temp, vector<vector<int>>& routingTable, PacketGenerator* packetGenerator, Client* client, CollisionAvoidance* AC, BlockingQueue< Message >*senderQueue){
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

void sendPing(BlockingQueue< Message >*senderQueue, Client* client, PacketGenerator* packetGenerator, CollisionAvoidance* AC){
	Message sendMessage;
	vector<Message> pingVector = packetGenerator->generatePingPacket(client);
	AC->sendMessageCA(pingVector, senderQueue);
	std::cout << std::endl << "Sent the first ping message!" << std::endl;
}


int main() {
	BlockingQueue< Message > receiverQueue; // Queue messages will arrive in
	BlockingQueue< Message > senderQueue; // Queue for data to transmit
	PacketGenerator packetGenerator;

	// Ask for address input. Should be between 0, 1, 2 or 3
	std::cout << "Please enter an address for this client (0, 1, 2 or 3)" << std::endl;
	bool input_valid = false;
	string addrInput;
	char my_addr = '4'; // initialized to wrong value so it has to be changed.

	// DVR variables
	bool tableConverged = false;
	bool sendRoutingTable = false;
	vector<vector<int>> routingTable = {{99, 99, 99, 99}, {99, 99, 99, 99}, {99, 99, 99, 99},{99, 99, 99, 99}};

	// Loop until valid input
	while(!input_valid){
		getline(cin,addrInput);
		my_addr = addrInput.at(0);
		if(my_addr == '0' || my_addr == '1' || my_addr == '2' || my_addr == '3'){
			input_valid = true;
		}
		else{
			std::cout << "Invalid input, please enter 0, 1, 2 or 3." << addrInput <<std::endl;
		}
	}


	Client client = Client(SERVER_ADDR, my_addr, SERVER_PORT, FREQUENCY, TOKEN, &senderQueue, &receiverQueue);
	CollisionAvoidance collisionAvoidance;
	
	client.startThread();
	
	// Sends the first discovery ping
	sendPing(&senderQueue, &client, &packetGenerator, &collisionAvoidance);

	thread inputHandler(readInput, &senderQueue, client.getMyAddr(), &collisionAvoidance, &client, &packetGenerator);
	thread routingTableSender(sendUpdatedTable, std::ref(routingTable), &packetGenerator, &client, &collisionAvoidance, &senderQueue, std::ref(sendRoutingTable));
	
	// Handle messages from the server / audio framework
	while(true){

		Message temp = receiverQueue.pop(); // wait for a message to arrive
		// std::cout << "Received: " << temp.type << std::endl; // print received chars
		collisionAvoidance.setReceivedMessageType(temp.type);

		// Go into the InitializeDVR state if table is not yet converged
		// if (tableConverged == false AND routingBit == 1)
		if (tableConverged == false){
			sendRoutingTable = routingMessageHandler(temp, routingTable, &packetGenerator, &client, &collisionAvoidance, &senderQueue);
			// ADD TIMEOUT FOR TABLECONVERGENCE
		}

		switch (temp.type) {
		case DATA: {// We received a data frame!
			std::cout << "DATA: ";
			for (char c : temp.data) {
				std::cout << c << ",";
			}
			if(((temp.data[0] & 0b00110000) >> 4) == (client.getMyAddr() -'0')){
				vector<Message> ackVector = packetGenerator.generateAckPacket((temp.data[1] & 0b111),&client,((temp.data[0] & 0b11000000) >> 6));
				bitset<8> tempdatazero((temp.data[0]>>6));
				bitset<8> seqNumSent((temp.data[1] & 0b111));
				std::cout << "Tempdatazero shifted: " << tempdatazero << std::endl;
				std::cout << "seqnumSent: " << seqNumSent << std::endl;
				senderQueue.push(ackVector[0]);	
			}
			std::cout << std::endl;
			break;
		}
		case DATA_SHORT:{ // We received a short data frame!
			std::cout << "DATA_SHORT: ";
			for (char c : temp.data) {
				std::cout << c << ",";
			}
			if(((temp.data[0] & 0b00110000) >> 4) == (client.getMyAddr() -'0')){

			
				bitset<8> shortReceived(temp.data[0]);
				bitset<8> shortReceivedScnd(temp.data[1]);
				std::cout << "First bit of ACK received: " << shortReceived << std::endl;
				std::cout << "2nd bit of ACK received: " << shortReceivedScnd << std::endl;
			}
			break;
		}
		case FREE: // The channel is no longer busy (no nodes are sending within our detection range)
			std::cout << "FREE" << std::endl;
			break;
		case BUSY: // The channel is busy (A node is sending within our detection range)
			std::cout << "BUSY" << std::endl;
			break;
		case SENDING: // This node is sending
			std::cout << "SENDING" << std::endl;
			break;
		case DONE_SENDING: // This node is done sending
			std::cout << "DONE_SENDING" << std::endl;
			break;
		case END: // Server / audio framework disconnect message. You don't have to handle this
			std::cout << "END" << std::endl;
			break;
		case HELLO: // Server / audio framework hello message. You don't have to handle this
			std::cout << "HELLO" << std::endl;
			break;
		case TOKEN_ACCEPTED: // Server / audio framework hello message. You don't have to handle this
			std::cout << "Token Valid!" << std::endl;
			break;
		case TOKEN_REJECTED: // Server / audio framework hello message. You don't have to handle this
			std::cout << "Token Rejected!" << std::endl;
			break;
		}
	}
	
}