#include <thread>
#include <string>
#include <cstring>
#include <vector>
#include <bitset>

#include "utils/BlockingQueue.h"
#include "network/Client.h"
#include "utils/Message.h"

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
void readInput(BlockingQueue< Message >*senderQueue, char addr) {
	while (true) {
		string input; 
		getline(cin, input); //read input from stdin for static address

		string finalInput = input + "0000000000000000000000000000000"; // zero padding
		finalInput.insert(0, 1, addr); // insert addr at front

		vector<char> char_vec(finalInput.begin(), finalInput.end()); // put input in char vector
		Message sendMessage;
		if (char_vec.size() > 2) {
			sendMessage = Message(DATA, char_vec);
		}
		else {
			sendMessage = Message(DATA_SHORT, char_vec);
		}		
		senderQueue->push(sendMessage); // put char vector in the senderQueue
	}
}


vector<vector<int>> initializeDVR(BlockingQueue< Message >*senderQueue, BlockingQueue< Message >*receiverQueue, char addr)  {
	// This function is used to find the initial topology
	// It outputs the completed initial lookup table
	
	// - Broadcast initial discover message, using CSMA/CD
	Message sendMessage;
	vector<vector<int>> routingTable;

	int sourceAddress = addr - '0';
	sourceAddress = sourceAddress << 6;		// Works up untill this point
	sourceAddress = sourceAddress + 0b00000000000000;
	
	// Constructing the initial message
	string output;
	output.insert(output.begin(), 1, sourceAddress);
	vector<char> char_vec(output.begin(), output.end());
	sendMessage = Message(DATA_SHORT, char_vec);
	senderQueue->push(sendMessage);

	// Add received discover messages to routingTable
	bool tableConverged = false;
	while(tableConverged == false){
		
		vector<char> data = temp.data;
		vector<uint8_t> bytes;
		unsigned int receivedSourceAddress;
		bool updatedTable = false;

		Message temp = receiverQueue->pop();

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
			// Add table information to own table
			
			break;
		}

		// If we received an update and this changes our table, send out new table to neighbours
		if (updatedTable = true){
		}
	}

	return routingTable;

}


int main() {
	BlockingQueue< Message > receiverQueue; // Queue messages will arrive in
	BlockingQueue< Message > senderQueue; // Queue for data to transmit

	string addrInput;
	getline(cin,addrInput);
	char my_addr = addrInput.at(0);

	Client client = Client(SERVER_ADDR, my_addr, SERVER_PORT, FREQUENCY, TOKEN, &senderQueue, &receiverQueue);

	client.startThread();

	// Starts DVR initialization process
	vector<vector<int>> routingTable;
	routingTable = initializeDVR(&senderQueue, &receiverQueue, client.getMyAddr());

	thread inputHandler(readInput, &senderQueue, client.getMyAddr());

	// Handle messages from the server / audio framework
	while(true){
		Message temp = receiverQueue.pop(); // wait for a message to arrive
		// cout << "Received: " << temp.type << endl; // print received chars
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