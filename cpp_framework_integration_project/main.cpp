#include <thread>
#include <string>
#include <cstring>
#include <vector>
#include <bitset>
#include <chrono>

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
int FREQUENCY = 8050;//TODO: Set this to your group frequency!
// The token you received for your frequency range
std::string TOKEN = "cpp-05-AYKI3U9SX758O0EPJT";


using namespace std;

void readInput(BlockingQueue< Message >*senderQueue, char addr, CollisionAvoidance* AC, Client* client, PacketGenerator* packetGenerator) {
while (true) {
		string input;
		cout << "Enter your message: " << endl;
		getline(cin, input); //read input from stdin
		if(input.size() < 16*30){
			vector<Message> packets = packetGenerator->generatePackets(input, client);
			while(packets.size()>0){
            	//assign first added Message to be send.
            	Message sendThisMessage = packets.front();
            	//pop the same message out of senderMessageVector.
	
            	//Je mag deze wel erasen, maar zorg ervoor dat hij eerst in een 
            	//andere vector<Message> opgeslagen wordt. Zodat pas bij een ACK hij daadwerkelijk loesoe is.
            	//EN tot die tijd evt. opnieuw gestuurd kan worden bij geen ACK.
            	packets.erase(packets.begin());
	
            	//run the checks
            	if((AC->queueIsBusy(AC->getReceivedMessageType().type)) == false){
            	    printf("ik was busy en kan nu sturen");
            	    senderQueue->push(sendThisMessage);
            	} else {
            	    printf("ik kan sws senden!");
            	    senderQueue->push(sendThisMessage);
            	}
        	}
		}
		else{
			cout << "Message too long, please write a shorter message!" << endl;
		}
	}
}
void sendUpdatedTable(vector<vector<int>> routingTable, PacketGenerator* PacketGenerator, Client* client){
	// This function can be called when a local routingtable has been updated
	// It sends the table to all neighbours, so that they can update theirs
	// It counts the width and height of the table, adds this to the first two bytes of data
	// Then it adds the table contents after that and sends the message over to the header constructor

	vector<char> sendingTable;

	cout << "sending the following table: " << endl;
	for (const auto& row : routingTable) {
        for (const auto& elem : row) {
            std::cout << elem << ' ';
        }
        std::cout << std::endl; // Newline for each row
    }
	// Add information about table size
	sendingTable[0] = routingTable.size();
	sendingTable[1] = routingTable[0].size();
	// Add table data
	for (int i = 0; i < routingTable.size(); i++){
		for (int j = 0; j < routingTable[0].size(); i++){
			sendingTable[i * routingTable[0].size() + j] = routingTable[i][j];
		}
	}
	
	vector<Message> routingVector = PacketGenerator->generateRoutingPacket(sendingTable,client);
	Message sendMessage;
	sendMessage = routingVector[0];
	// THIS MESSAGE IS NOT YET SENT
}

void routingMessageHandler(Message temp, vector<vector<int>>& routingTable, PacketGenerator* packetGenerator, Client* client){
	// Input: routing message, local routing table
	// Takes a message that has been flagged as "routing" and processes it
	// It checks if the local routingtable has to be updated, and calls the 
	// sendUpdatedTable function if true.
	vector<char> data = temp.data;
	vector<uint8_t> bytes;
	vector<uint8_t> receivedTableData;
	vector<vector<uint8_t>> receivedTable;
	unsigned int receivedSourceAddress;
	int dataLength = 0;
	int tableDests = 0;
	int tableVias = 0;
	bool updatedTable = false;

	switch(temp.type) {
		case DATA_SHORT:	//This means that it is a discovery message
		// Add neighbour to own tablex
		for (char c : data) {
			bytes.push_back(static_cast<unsigned int>(c));
		}
		receivedSourceAddress = bytes[0] >> 6;
		cout << "The received source address is: " << receivedSourceAddress << endl;
		if (routingTable[receivedSourceAddress][receivedSourceAddress] != 1){
			routingTable[receivedSourceAddress][receivedSourceAddress] = 1;
			updatedTable = true;
			cout << "Updated the routing table to " << routingTable[receivedSourceAddress][receivedSourceAddress] << endl;
		}
		break;
		case DATA:			// This means that it is a topology message
		// Import and process incoming table
		for (char c : data) {
			if (c != '\0'){
				dataLength++;	// Finds the amount of data in the packet
			}
			bytes.push_back(static_cast<unsigned int>(c));	
		}
		receivedSourceAddress = bytes[0] >> 6;
		// Find table dimensions
		tableDests = bytes[2];
		tableVias = bytes[3];

		// Remove header from data
		for (int i = 4; i < dataLength; i++){
			receivedTableData[i-4] = bytes[i];
		}
		
		// Sort data into 2D table TODO: CHECK IF ORDER IS CORRECT
		for (int i = 0; i < tableDests; i++){
			for (int j = 0; j < tableVias; j++){
				receivedTable[i][j] = receivedTableData[i * tableVias + j];
			}
		}

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
		break;
	}
	
	if (updatedTable == true){
		cout << "I want to send out my table!" << endl;
		sendUpdatedTable(routingTable,packetGenerator,client, );
		updatedTable = false;
	}
	// TODO:
	// - Routing table update functies apart maken
	// - Routing table verzend functie apart maken
}

vector<vector<int>> initializeDVR(BlockingQueue< Message >*senderQueue, BlockingQueue< Message >*receiverQueue, char addr, Client* client, PacketGenerator* packetGenerator)  {
	// This function is used to find the initial topology
	// It outputs the completed initial lookup table
	
	// - Broadcast initial discover message, using CSMA/CD
	Message sendMessage;
	// Initialize the routingtable with "infinite"
	vector<vector<int>> routingTable = {{99, 99, 99, 99}, {99, 99, 99, 99}, {99, 99, 99, 99},{99, 99, 99, 99}};

	vector<Message> pingVector = packetGenerator->generatePingPacket(client);

	sendMessage = pingVector[0];
	senderQueue->push(sendMessage); // HAVE TO ADD THE HEADER AND SENDING FUNCTION HERE, ADD ROUTING FLAG AS WELL
	cout << "Sent the first ping message!" << endl;

	// Add received discover messages to routingTable
	bool tableConverged = false;
	while(tableConverged == false){
		Message temp = receiverQueue->pop();
		routingMessageHandler(temp, routingTable, packetGenerator, client);

		// While no new packet in queue, update timer,
		// if timer is higher than treshhold
		// chrono::steady_clock::time_point start = chrono::steady_clock::now();
		// while (receiverQueue->isempty() == true){
		// 	if (chrono::steady_clock::now() - start >= timeout){
		// 		tableConverged = true;
		// 		break;
		// 	}
		// }
	}
	
	return routingTable;
}
int main() {
	BlockingQueue< Message > receiverQueue; // Queue messages will arrive in
	BlockingQueue< Message > senderQueue; // Queue for data to transmit
	PacketGenerator packetGenerator;

	// Ask for address input. Should be between 0, 1, 2 or 3
	cout << "Please enter an address for this client (0, 1, 2 or 3)" << endl;
	bool input_valid = false;
	string addrInput;
	char my_addr = '4'; // initialized to wrong value so it has to be changed.

	// Loop until valid input
	while(!input_valid){
		getline(cin,addrInput);
		my_addr = addrInput.at(0);
		if(my_addr == '0' || my_addr == '1' || my_addr == '2' || my_addr == '3'){
			input_valid = true;
		}
		else{
			cout << "Invalid input, please enter 0, 1, 2 or 3." << addrInput <<endl;
		}
	}

	Client client = Client(SERVER_ADDR, my_addr, SERVER_PORT, FREQUENCY, TOKEN, &senderQueue, &receiverQueue);
	CollisionAvoidance collisionAvoidance;
	
	client.startThread();

	// Starts DVR initialization process
	vector<vector<int>> routingTable;
	routingTable = initializeDVR(&senderQueue, &receiverQueue, client.getMyAddr(),&client,&packetGenerator);


	thread inputHandler(readInput, &senderQueue, client.getMyAddr(), &collisionAvoidance, &client, &packetGenerator);
	
	// Handle messages from the server / audio framework
	while(true){
		Message temp = receiverQueue.pop(); // wait for a message to arrive
		// cout << "Received: " << temp.type << endl; // print received chars
		collisionAvoidance.setReceivedMessageType(temp.type);
		switch (temp.type) {
		case DATA: {// We received a data frame!
			cout << "DATA: ";
			for (char c : temp.data) {
				cout << c << ",";
			}
			if(((temp.data[0] & 0b00110000) >> 4) == (client.getMyAddr() -'0')){
				vector<Message> ackVector = packetGenerator.generateAckPacket((temp.data[1] & 0b111),&client,((temp.data[0] & 0b11000000) >> 6));
				bitset<8> tempdatazero((temp.data[0]>>6));
				bitset<8> seqNumSent((temp.data[1] & 0b111));
				cout << "Tempdatazero shifted: " << tempdatazero << endl;
				cout << "seqnumSent: " << seqNumSent << endl;
				senderQueue.push(ackVector[0]);	
			}
			cout << endl;
			break;
		}
		case DATA_SHORT:{ // We received a short data frame!
			cout << "DATA_SHORT: ";
			for (char c : temp.data) {
				cout << c << ",";
			}
			if(((temp.data[0] & 0b00110000) >> 4) == (client.getMyAddr() -'0')){

			
				bitset<8> shortReceived(temp.data[0]);
				bitset<8> shortReceivedScnd(temp.data[1]);
				cout << "First bit of ACK received: " << shortReceived << endl;
				cout << "2nd bit of ACK received: " << shortReceivedScnd << endl;
			}
			break;
		}
		case FREE: // The channel is no longer busy (no nodes are sending within our detection range)
			cout << "FREE" << endl;
			break;
		case BUSY: // The channel is busy (A node is sending within our detection range)
			cout << "BUSY" << endl;
			break;
		case SENDING: // This node is sending
			cout << "SENDING" << endl;
			break;
		case DONE_SENDING: // This node is done sending
			cout << "DONE_SENDING" << endl;
			break;
		case END: // Server / audio framework disconnect message. You don't have to handle this
			cout << "END" << endl;
			break;
		case HELLO: // Server / audio framework hello message. You don't have to handle this
			cout << "HELLO" << endl;
			break;
		case TOKEN_ACCEPTED: // Server / audio framework hello message. You don't have to handle this
			cout << "Token Valid!" << endl;
			break;
		case TOKEN_REJECTED: // Server / audio framework hello message. You don't have to handle this
			cout << "Token Rejected!" << endl;
			break;
		}
	}
	
}