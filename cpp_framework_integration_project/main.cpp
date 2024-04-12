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
		cout << "Enter your message: " << endl;
		getline(cin, input); //read input from stdin


		/// Add header
		int firstByte = 0;
		int senderAddress = addr - '0'; // This gives the true integer value (0, 1, 2 or 3)

		// Set first 2 bits to be the source address
		firstByte = senderAddress << 6; 

		// Set second 2 bits to be destination address
		int destAddress = 0b11; // TO-DO: implement dynamic destination address
		firstByte = firstByte | (destAddress << 4);

		bitset<8> checkFirstByte(firstByte);
		cout << "First byte in binary = " << checkFirstByte << endl;


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

	client.startThread();

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