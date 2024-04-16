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
int FREQUENCY = 8090;//TODO: Set this to your group frequency!
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
			//Send message via Collision Avoidance
			AC->sendMessageCA(packets, senderQueue);
		}
		else{
			cout << "Message too long, please write a shorter message!" << endl;
		}
	}
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
			vector<Message> ackVector = packetGenerator.generateAckPacket((temp.data[1] & 0b111),&client,((temp.data[0] & 0b11000000) >> 6));
			bitset<8> tempdatazero((temp.data[0]>>6));
			bitset<8> seqNumSent((temp.data[1] & 0b111));
			cout << "Tempdatazero shifted: " << tempdatazero << endl;
			cout << "seqnumSent: " << seqNumSent << endl;
			// senderQueue.push(ackVector[0]);
			collisionAvoidance.sendMessageCA(ackVector, &senderQueue);
			cout << endl;
			break;
		}
		case DATA_SHORT:{ // We received a short data frame!
			cout << "DATA_SHORT: ";
			for (char c : temp.data) {
				cout << c << ",";
			}
			bitset<8> shortReceived(temp.data[0]);
			bitset<8> shortReceivedScnd(temp.data[1]);
			cout << "First bit of ACK received: " << shortReceived << endl;
			cout << "2nd bit of ACK received: " << shortReceivedScnd << endl;
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