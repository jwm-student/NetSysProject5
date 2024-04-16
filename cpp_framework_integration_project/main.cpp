#include <thread>
#include <string>
#include <cstring>
#include <vector>
#include <bitset>
#include <chrono>
#include <stdlib.h>

#include "utils/BlockingQueue.h"
#include "network/Client.h"
#include "utils/Message.h"
#include "network/CollisionAvoidance.h"

/**
* This is just some example code to show you how to interact 
* with the server using the provided client and two queues.
* Feel free to modify this code in any way you like! l
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
vector<Message> packetGenerator(string input, Client* client){
	vector<Message> output;
	// Vector for padding
	vector<char> zeroVector = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};

	/// Add header
	int senderAddress = client->getMyAddr() - '0'; // This gives the true integer value (0, 1, 2 or 3)
	// Set first 2 bits to be the source address
	int firstByte = senderAddress << 6; 

	// Set second 2 bits to be destination address
	int destAddress = 0b11; // TO-DO: implement dynamic destination address
	firstByte = firstByte | (destAddress << 4);

	// After source and destination address there are 4 bits that represent the data offset.
	// If the length of the message is less than 30 bytes, these are set to 0, and if this is the case, 
	// nothing needs to be done about the last 4 bits of the first byte
	if(input.length() > 30){
		// Start sending multiple messages
		int msgLength = input.length();
		int bytesSent = 0;
		// Keep sending until you should have sent all bits
		while(bytesSent < msgLength){
			int dataOffset = bytesSent / 30;
			firstByte = firstByte | dataOffset; // Update firstByte with Offset.

			int secondByte = 0;
			// If there are more than 30 bits to send still, its not the last package.
			if((msgLength-bytesSent) > 30){
				secondByte = 1 << 6; // Sets the flag bit (2nd bit from left) to indicate it is not the last pkt.
				input.insert(input.end(),zeroVector.begin(),zeroVector.end()); // Append 0 vector
			}
			secondByte = secondByte | client->getSeqNum();

			// Create final pkt
			vector<char> char_vec;
			char_vec.push_back(firstByte);
			char_vec.push_back(secondByte);
			// Fetch data to be send from input
			for(int i = 0; i < 30; i++){
				char_vec.push_back(input[bytesSent+i]);
			}

			Message sendMessage = Message(DATA, char_vec);
			output.push_back(sendMessage);
			bytesSent += 30;
			client->increaseSeqNum();
		}
	}
	else{
		// The message fits in one data packet
		// Create 2nd Header Byte
		int secondByte = client->getSeqNum();

		input.insert(0, 1, (char)firstByte); // insert firstByte at front, with src and dst address and data offset set to 0
		input.insert(1, 1, (char)secondByte); // insert secondByte as the second byte, with the sequence number.

		vector<char> char_vec(input.begin(), input.end()); // put input in char vector
		Message sendMessage;
		if (char_vec.size() > 2) {
			// TO-DO: See if just a for loop pushing back each char of the input works better
			char_vec.insert(char_vec.end(),zeroVector.begin(),zeroVector.end()); // Append 0 vector
			sendMessage = Message(DATA, char_vec);
			output.push_back(sendMessage);
		}
		else {
			sendMessage = Message(DATA_SHORT, char_vec);
			output.push_back(sendMessage);
		}
		client->increaseSeqNum();
	}
	return output;
}

void readInput(BlockingQueue< Message >*senderQueue, char addr, CollisionAvoidance* AC, Client* client) {
	while (true) {
		string input;
		cout << "Enter your message: " << endl;
		getline(cin, input); //read input from stdin
		if(input.size() < 16*30){
			vector<Message> packets = packetGenerator(input, client);
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
		else {
			cout << "Message too long, please write a shorter message!" << endl;
		}
	}
}

void sendUpdatedTable(vector<vector<int>> routingTable){
	// This function can be called when a local routingtable has been updated
	// It sends the table to all neighbours, so that they can update theirs
	// It counts the width and height of the table, adds this to the first two bytes of data
	// Then it adds the table contents after that and sends the message over to the header constructor

	vector<char> sendingTable;
	// Add information about table size
	sendingTable[0] = routingTable.size();
	sendingTable[1] = routingTable[0].size();
	// Add table data
	for (int i = 0; i < routingTable.size(); i++){
		for (int j = 0; j < routingTable[0].size(); i++){
			sendingTable[i * routingTable[0].size() + j] = routingTable[i][j];
		}
	}

	// THIS DOES NOT YET ADD HEADERS OR ZERO PADDING, HAND OVER TO PACKAGING FUNCTION TO FIX
	Message sendMessage;
	sendMessage = Message(DATA, sendingTable);
}

void routingMessageHandler(Message temp, vector<vector<int>>& routingTable){
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
		sendUpdatedTable(routingTable);
		updatedTable = false;
	}
	// TODO:
	// - Routing table update functies apart maken
	// - Routing table verzend functie apart maken
}

vector<vector<int>> initializeDVR(BlockingQueue< Message >*senderQueue, BlockingQueue< Message >*receiverQueue, char addr)  {
	// This function is used to find the initial topology
	// It outputs the completed initial lookup table
	
	// - Broadcast initial discover message, using CSMA/CD
	Message sendMessage;
	vector<vector<int>> routingTable;

	int sourceAddress = addr - '0';
	sourceAddress = sourceAddress << 6;
	sourceAddress = sourceAddress + 0b00000000000000;
	
	// Constructing the initial message
	string output;
	output.insert(output.begin(), 1, sourceAddress);
	vector<char> char_vec(output.begin(), output.end());
	sendMessage = Message(DATA_SHORT, char_vec);
	senderQueue->push(sendMessage); // HAVE TO ADD THE HEADER AND SENDING FUNCTION HERE, ADD ROUTING FLAG AS WELL

	// Add received discover messages to routingTable
	bool tableConverged = false;
	while(tableConverged == false){
		
		chrono::milliseconds timeout(5000);

		Message temp = receiverQueue->pop();
		routingMessageHandler(temp, routingTable);

		// While no new packet in queue, update timer,
		// if timer is higher than treshhold
		chrono::steady_clock::time_point start = chrono::steady_clock::now();
		while (receiverQueue->isempty() == true){
			if (chrono::steady_clock::now() - start >= timeout){
				tableConverged = true;
				break;
			}
		}

	}
	
	return routingTable;
}

void sendUpdatedTable(vector<vector<int>> routingTable){
	// This function can be called when a local routingtable has been updated
	// It sends the table to all neighbours, so that they can update theirs
	// It counts the width and height of the table, adds this to the first two bytes of data
	// Then it adds the table contents after that and sends the message over to the header constructor

	vector<char> sendingTable;
	// Add information about table size
	sendingTable[0] = routingTable.size();
	sendingTable[1] = routingTable[0].size();
	// Add table data
	for (int i = 0; i < routingTable.size(); i++){
		for (int j = 0; j < routingTable[0].size(); i++){
			sendingTable[i * routingTable[0].size() + j] = routingTable[i][j];
		}
	}

	// THIS DOES NOT YET ADD HEADERS OR ZERO PADDING, HAND OVER TO PACKAGING FUNCTION TO FIX
	Message sendMessage;
	sendMessage = Message(DATA, sendingTable);
}

void routingMessageHandler(Message temp, vector<vector<int>>& routingTable){
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
		sendUpdatedTable(routingTable);
		updatedTable = false;
	}
	// TODO:
	// - Routing table update functies apart maken
	// - Routing table verzend functie apart maken
}

vector<vector<int>> initializeDVR(BlockingQueue< Message >*senderQueue, BlockingQueue< Message >*receiverQueue, char addr)  {
	// This function is used to find the initial topology
	// It outputs the completed initial lookup table
	
	// - Broadcast initial discover message, using CSMA/CD
	Message sendMessage;
	vector<vector<int>> routingTable;

	int sourceAddress = addr - '0';
	sourceAddress = sourceAddress << 6;
	sourceAddress = sourceAddress + 0b00000000000000;
	
	// Constructing the initial message
	string output;
	output.insert(output.begin(), 1, sourceAddress);
	vector<char> char_vec(output.begin(), output.end());
	sendMessage = Message(DATA_SHORT, char_vec);
	senderQueue->push(sendMessage); // HAVE TO ADD THE HEADER AND SENDING FUNCTION HERE, ADD ROUTING FLAG AS WELL

	// Add received discover messages to routingTable
	bool tableConverged = false;
	while(tableConverged == false){
		
		chrono::milliseconds timeout(5000);

		Message temp = receiverQueue->pop();
		routingMessageHandler(temp, routingTable);

		// While no new packet in queue, update timer,
		// if timer is higher than treshhold
		chrono::steady_clock::time_point start = chrono::steady_clock::now();
		while (receiverQueue->isempty() == true){
			if (chrono::steady_clock::now() - start >= timeout){
				tableConverged = true;
				break;
			}
		}

	}
	
	return routingTable;
}

int main() {
	BlockingQueue< Message > receiverQueue; // Queue messages will arrive in
	BlockingQueue< Message > senderQueue; // Queue for data to transmit

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
	routingTable = initializeDVR(&senderQueue, &receiverQueue, client.getMyAddr());

	// Starts DVR initialization process
	vector<vector<int>> routingTable;
	routingTable = initializeDVR(&senderQueue, &receiverQueue, client.getMyAddr());

	thread inputHandler(readInput, &senderQueue, client.getMyAddr(), &collisionAvoidance, &client);
	
	// Handle messages from the server / audio framework
	while(true){
		Message temp = receiverQueue.pop(); // wait for a message to arrive
		// cout << "Received: " << temp.type << endl; // print received chars
		collisionAvoidance.setReceivedMessageType(temp.type);
		switch (temp.type) {
		case DATA: // We received a data frame!
			cout << "DATA: ";
			for (char c : temp.data) {
				cout << c << ",";
			}
			cout << endl;
			break;
		case DATA_SHORT: // We received a short data frame!
			cout << "DATA_SHORT: ";
			for (char c : temp.data) {
				cout << c << ",";
			}
			cout << endl;
			break;
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