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
int FREQUENCY = 8050;//TODO: Set this to your group frequency!
// The token you received for your frequency range
std::string TOKEN = "cpp-05-AYKI3U9SX758O0EPJT";

using namespace std;
void readInput(BlockingQueue< Message >*senderQueue, char addr, Client* client) {
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

		// Print out byte
		bitset<8> checkFirstByte(firstByte);
		cout << "First byte in binary = " << checkFirstByte << endl;

		// After source and destination address there are 4 bits that represent the data offset.
		// If the length of the message is less than 29 bytes, these are set to 0, and if this is the case, 
		// nothing needs to be done about the last 4 bits of the first byte
		if(input.length() > 30){
			// Start sending multiple messages
			int msgLength = input.length();
			int bytesSent = 0;

			// If the message is too long, the code will break so user will have to send shorter message.
			if (msgLength > (16*30)){
				cout << "message too long, please send a shorter message" << endl;
				continue;
			}
			else{
			// Keep sending until you should have sent all bits
				while(bytesSent < msgLength){
					int dataOffset = bytesSent / 30;

					// If there are more than 30 bits to send still, its not the last package.
					firstByte = firstByte | dataOffset; // Update firstByte with Offset.
					bitset<8> checkFirstByte(firstByte);
					cout << "First byte in binary (arbmsg) = " << checkFirstByte << endl;

					int secondByte = 0;
					if((msgLength-bytesSent) > 30){
						cout << "still more to send, flag bit set to 1!" << endl;
						secondByte = 1 << 6; // Sets the flag bit (2nd bit from left) to indicate it is not the last pkt.
					}
					secondByte = secondByte | client->getSeqNum();
					bitset<8> arbMsgSecondByte(secondByte);
					cout << "Scnd byte in binary (arbmsg) = " << arbMsgSecondByte << endl;

					// Create final pkt
					vector<char> char_vec;
					char_vec.push_back(firstByte);
					char_vec.push_back(secondByte);
					// Fetch data to be send from input
					for(int i = 0; i < 30; i++){
						char_vec.push_back(input[bytesSent+i]);
					}

					Message sendMessage = Message(DATA, char_vec);
					senderQueue->push(sendMessage); // put char vector in the senderQueue
					bytesSent += 30;
					client->increaseSeqNum();
				}
			}
		}
		else{
			// The message fits in one data packet
			// Create 2nd Header Byte
			int secondByte = client->getSeqNum();
			bitset<8> checkSecondByte(secondByte);
			cout << "second byte in binary = " << checkSecondByte << endl;

			input.insert(0, 1, (char)firstByte); // insert firstByte at front, with src and dst address and data offset set to 0
			input.insert(1, 1, (char)secondByte); // insert secondByte as the second byte, with the sequence number.
			cout << "this is now the input:" << input << endl;
			vector<char> char_vec(input.begin(), input.end()); // put input in char vector
			Message sendMessage;
			if (char_vec.size() > 2) {
				// Zero 
				vector<char> zeroVector = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
				char_vec.insert(char_vec.end(),zeroVector.begin(),zeroVector.end()); // Append 0 vector

				sendMessage = Message(DATA, char_vec);
			}
			else {
				sendMessage = Message(DATA_SHORT, char_vec);
			}		
			senderQueue->push(sendMessage); // put char vector in the senderQueue
			client->increaseSeqNum();
		}
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

	thread inputHandler(readInput, &senderQueue, client.getMyAddr(), &client);
	
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