#include <thread>
#include <string>
#include <cstring>
#include <vector>
#include <bitset>
#include <chrono>
#include <functional>

#include "utils/TUI.h"
#include "utils/BlockingQueue.h"
#include "network/Client.h"
#include "utils/Message.h"
#include "network/CollisionAvoidance.h"
#include "network/PacketGenerator.hpp"
#include "network/DVR.h"
#include "network/PacketProcessor.hpp"

// The address to connect to. Set this to localhost to use the audio interface tool.
std::string SERVER_ADDR = "netsys.ewi.utwente.nl"; //"127.0.0.1"
// The port to connect to. 8954 for the simulation server
int SERVER_PORT = 8954;
// The frequency to connect on.
int FREQUENCY = 8000;//TODO: Set this to your group frequency!
// The token you received for your frequency range
std::string TOKEN = "cpp-05-AYKI3U9SX758O0EPJT";


using namespace std;

void readInput(TUI *tui) {
	while (true) {
		cout << "Enter your command: " << endl;
		string input;
		getline(cin, input);
		tui->processInput(input);
	}
}


int main() {
	BlockingQueue< Message > receiverQueue; // Queue messages will arrive in
	BlockingQueue< Message > senderQueue; // Queue for data to transmit	

	// Ask for address input. Should be between 0, 1, 2 or 3
	std::cout << "Please enter an address for this client (0, 1, 2 or 3)" << std::endl;
	bool input_valid = false;
	string addrInput;
	int my_addr = 4; // initialized to wrong value so it has to be changed.
	
	// DVR variables
	bool tableConverged = false;
	bool sendRoutingTable = false;
	vector<vector<int>> routingTable = {{99, 99, 99, 99}, {99, 99, 99, 99}, {99, 99, 99, 99},{99, 99, 99, 99}};
	chrono::milliseconds routingTimeout(10000);

	//Initializing classes.
	Client client = Client(SERVER_ADDR, my_addr, SERVER_PORT, FREQUENCY, TOKEN, &senderQueue, &receiverQueue);
	CollisionAvoidance collisionAvoidance(&senderQueue);
	PacketGenerator packetGenerator(&client);
	TUI tui = TUI(&client, &packetGenerator, &collisionAvoidance);
	
	client.setMyAddr(tui.setDestinationAddress()); // set address to input of user

	client.startThread();
	

	PacketProcessor PP(&packetGenerator, &collisionAvoidance, &client);
	
	DVR DVR(&senderQueue, &client, &packetGenerator, &collisionAvoidance);
	// Sends the first discovery ping
	DVR.sendPing();

	thread inputHandler(readInput, &tui);
	thread routingTableSender(std::bind(&DVR::sendUpdatedTable, &DVR, std::ref(routingTable), std::ref(sendRoutingTable)));
	//std::ref(sendRoutingTable)

	routingTable[my_addr][my_addr] = 0;
	chrono::steady_clock::time_point start = chrono::steady_clock::now();
	
	//Handle messages from the server / audio framework
	while(true){

		Message temp = receiverQueue.pop(); // wait for a message to arrive
		// std::cout << "Received: " << temp.type << std::endl; // print received chars
		collisionAvoidance.setReceivedMessageType(temp.type);

		// Go into the InitializeDVR state if table is not yet converged
		// if (tableConverged == false AND routingBit == 1)
		
		if (tableConverged == false){
			std::cout << "entered tableConverged" << std::endl;
			while (receiverQueue.isempty() == true){
				if (chrono::steady_clock::now() - start >= routingTimeout){
					tableConverged = true;
					std::cout << "tableConverged timed out!" << std::endl;
					break;
				}
			}
			if (receiverQueue.isempty() == false){
				sendRoutingTable = DVR.routingMessageHandler(temp, routingTable);
				chrono::steady_clock::time_point start = chrono::steady_clock::now();
			}

			// ADD TIMEOUT FOR TABLECONVERGENCE
		}

		
		switch (temp.type) {
		case DATA: {// We received a data frame!
			std::cout << "DATA: ";
			for (char c : temp.data) {
				std::cout << c << ",";
			}
			PP.processDataPacket(temp);
			// if(((temp.data[0] & 0b00110000) >> 4) == (client.getMyAddr() -'0')){
			// 	vector<Message> ackVector = packetGenerator.generateAckPacket((temp.data[1] & 0b111),&client,((temp.data[0] & 0b11000000) >> 6));
			// 	bitset<8> tempdatazero((temp.data[0]>>6));
			// 	bitset<8> seqNumSent((temp.data[1] & 0b111));
			// 	std::cout << "Tempdatazero shifted: " << tempdatazero << std::endl;
			// 	std::cout << "seqnumSent: " << seqNumSent << std::endl;
			// 	senderQueue.push(ackVector[0]);	
			// }
			// std::cout << std::endl;
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