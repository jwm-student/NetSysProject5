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
#include "network/DVR.h"
#include "network/PacketProcessor.hpp"

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
int FREQUENCY = 8010;//TODO: Set this to your group frequency!
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
	PacketProcessor PP(&packetGenerator, &collisionAvoidance, &client);
	
	client.startThread();
	
	DVR DVR(&senderQueue, &client, &packetGenerator, &collisionAvoidance);
	// Sends the first discovery ping
	DVR.sendPing();

	thread inputHandler(readInput, &senderQueue, client.getMyAddr(), &collisionAvoidance, &client, &packetGenerator);
	thread routingTableSender(std::bind(&DVR::sendUpdatedTable, &DVR, std::ref(routingTable), std::ref(sendRoutingTable)));
	//std::ref(sendRoutingTable)

	unsigned int my_addr_int = my_addr - '0';
	std::cout << "My address is: " << my_addr_int << std::endl;
	routingTable[my_addr_int][my_addr_int] = 0;
	
	//Handle messages from the server / audio framework
	while(true){

		Message temp = receiverQueue.pop(); // wait for a message to arrive
		// std::cout << "Received: " << temp.type << std::endl; // print received chars
		collisionAvoidance.setReceivedMessageType(temp.type);

		// Go into the InitializeDVR state if table is not yet converged
		// if (tableConverged == false AND routingBit == 1)
		if (tableConverged == false){
			sendRoutingTable = DVR.routingMessageHandler(temp, routingTable);
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