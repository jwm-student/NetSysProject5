#include <thread>
#include <string>
#include <cstring>
#include <vector>
#include <chrono>

#include "utils/BlockingQueue.h"
#include "network/Client.h"
#include "utils/Message.h"
#include "network/CollisionAvoidance.h"

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
void readInput(BlockingQueue< Message >*senderQueue, char addr, CollisionAvoidance* AC) {
	while (true) {
		string input;
		getline(cin, input); //read input from stdin

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
		//Deze functie moet recursive zijn. Dan fix je het probleem dat je wel collide bij 3 of meer nodes.
		//DO not send if busy
		if(AC->getReceivedMessageType().type == BUSY){
			while(BUSY){
				int rn = (rand() % 50);
				// sleep for random ms
				// recheck if newType is not BUSY
				if(!(AC->getReceivedMessageType().type == BUSY)){
					std::cout<< "slept for " << rn << std::endl;
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(rn));

			}
			//if out of BUSY
			//try to resend message
			printf(" uit de while busy loop ");
			senderQueue->push(sendMessage); // put char vector in the senderQueue
		} else {
			printf("stuur dit als je als eerste mag anyway");
			senderQueue->push(sendMessage); // put char vector in the senderQueue
		}
	}
}

int main() {
	BlockingQueue< Message > receiverQueue; // Queue messages will arrive in
	BlockingQueue< Message > senderQueue; // Queue for data to transmit
	printf("set an address integer between 0 - 3 ");
	string addrInput;
	getline(cin,addrInput);
	char my_addr = addrInput.at(0);

	Client client = Client(SERVER_ADDR, my_addr, SERVER_PORT, FREQUENCY, TOKEN, &senderQueue, &receiverQueue);
	CollisionAvoidance collisionAvoidance;
	
	client.startThread();

	thread inputHandler(readInput, &senderQueue, client.getMyAddr(), &collisionAvoidance);
	
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