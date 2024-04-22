#include <iostream>
#include <string>
#include <cstring>
#include <bitset>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#elif __linux__
#include <sys/socket.h>    	//socket
#include <arpa/inet.h> 		//inet_addr
#endif

#include "PacketGenerator.hpp"

PacketGenerator::PacketGenerator(Client* client, vector<vector<int>>* routingTable){
    this->client = client;
    this->routingTable = routingTable;
}

vector<Message> PacketGenerator::generatePackets(std::string input){
    vector<Message> output;
    if(input.size() > 30){
        output = generateMultiPacket(input);
    }
    else{
        output = generateSingleDataPacket(input);
    }
    return output;
}

vector<Message> PacketGenerator::generatePackets(std::string input, int destAddr){
    vector<Message> output;
    if(input.size() > 30){
        if(destAddr != findNextHop(destAddr)){ // If it is not a direct neighbour, make sure nextHop is added
            vector<Message> notYetOutput = generateMultiPacket(input, destAddr);
            for(auto m : notYetOutput){
                output.push_back(generateNextHopPacket(m,destAddr)); // Change each message to message with nexthop
            }
        }
        else{
            output = generateMultiPacket(input,destAddr);
        }
    }
    else{
        if(destAddr != findNextHop(destAddr)){ // If it is not a direct neighbour, make sure nextHop is added
            vector<Message> notYetOutput = generateSingleDataPacket(input, destAddr);
            output.push_back(generateNextHopPacket(notYetOutput[0],destAddr)); // Change each message to message with nexthop
        }
        else{
            output = generateSingleDataPacket(input, destAddr);
        }
    }
    return output;
}

vector<Message> PacketGenerator::generateAckPacket(int seqNum, int destAddr){
    vector<Message> output;

    // Set first 2 bits to be the source address
	int senderAddress = client->getMyAddr(); // This gives the true integer value (0, 1, 2 or 3)
	int firstByte = senderAddress << 6; 
    bitset<8> sendAddr(senderAddress);
    std::cout << "Sender address in bits: " << sendAddr << std::endl;
	// Set second 2 bits to be destination address
	firstByte = firstByte | (destAddr << 4);
    bitset<8> ackSent(firstByte);
    std::cout << "destAddr: " << destAddr << std::endl;
    std::cout << "First bit of ACK sent: " << ackSent << std::endl;

    // Create second byte
    int secondByte = seqNum;

    //TODO: Impelement next hop when relevant!
    vector<char> char_vec; // put input in char vector
    char_vec.push_back(firstByte);
    char_vec.push_back(secondByte);

    Message sendMessage;
    sendMessage = Message(DATA_SHORT, char_vec);
    output.push_back(sendMessage);

    client->increaseExpSeqNum();
	return output;
}

vector<Message> PacketGenerator::generateMultiPacket(std::string input){
    vector<Message> output;
    // Vector for padding
	vector<char> zeroVector = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};

	/// Add header
	int senderAddress = client->getMyAddr(); // This gives the true integer value (0, 1, 2 or 3)
	// Set first 2 bits to be the source address
	int firstByte = senderAddress << 6; 

	// Set second 2 bits to be destination address
	int destAddress = 0b11; // TO-DO: implement dynamic destination address
	firstByte = firstByte | (destAddress << 4);

	// After source and destination address there are 4 bits that represent the data offset.
	// If the length of the message is less than 30 bytes, these are set to 0, and if this is the case, 
	// nothing needs to be done about the last 4 bits of the first byte
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
    return output;
}
vector<Message> PacketGenerator::generateMultiPacket(std::string input, int destAddr){
    vector<Message> output;
    // Vector for padding
	vector<char> zeroVector = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};

	/// Add header
	int senderAddress = client->getMyAddr(); // This gives the true integer value (0, 1, 2 or 3)
	// Set first 2 bits to be the source address
	int firstByte = senderAddress << 6; 

	// Set second 2 bits to be destination address
	firstByte = firstByte | (destAddr << 4);

	// After source and destination address there are 4 bits that represent the data offset.
	// If the length of the message is less than 30 bytes, these are set to 0, and if this is the case, 
	// nothing needs to be done about the last 4 bits of the first byte
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
    return output;
}

vector<Message> PacketGenerator::generateSingleDataPacket(std::string input){
    vector<Message> output;
	// Vector for padding
	vector<char> zeroVector = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};

	/// Add header
	int senderAddress = client->getMyAddr(); // This gives the true integer value (0, 1, 2 or 3)
	// Set first 2 bits to be the source address
	int firstByte = senderAddress << 6; 

	// Set second 2 bits to be destination address
	int destAddress = 0b11; // TO-DO: implement dynamic destination address
	firstByte = firstByte | (destAddress << 4);
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
	return output;
}

vector<Message> PacketGenerator::generateSingleDataPacket(std::string input, int destAddr){
    vector<Message> output;
	// Vector for padding
	vector<char> zeroVector = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};

	/// Add header
	int senderAddress = client->getMyAddr(); // This gives the true integer value (0, 1, 2 or 3)
	// Set first 2 bits to be the source address
	int firstByte = senderAddress << 6; 

	// Set second 2 bits to be destination address
	firstByte = firstByte | (destAddr << 4);
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
	return output;
}

vector<Message> PacketGenerator::generatePingPacket(Client* client){
    vector<Message> output;

    // Set first 2 bits to be the source address
	int senderAddress = client->getMyAddr(); // This gives the true integer value (0, 1, 2 or 3)
	int firstByte = senderAddress << 6; 

    bitset<8> sendAddr(senderAddress);
    std::cout << "Sender address in bits: " << sendAddr << std::endl;
	// Set second 2 bits to be destination address

    // Create second byte
    int secondByte = 0b10000000;

    //TODO: Impelement next hop when relevant!
    vector<char> char_vec; // put input in char vector
    char_vec.push_back(firstByte);
    char_vec.push_back(secondByte);

    Message sendMessage;
    sendMessage = Message(DATA_SHORT, char_vec);
    output.push_back(sendMessage);

	return output;
}

vector<Message> PacketGenerator::generateRoutingPacket(vector<char> sendingTable, Client* client){
    vector<Message> output;
	// Vector for padding
	vector<char> zeroVector = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};

	/// Add header
	int senderAddress = client->getMyAddr(); // This gives the true integer value (0, 1, 2 or 3)
	// Set first 2 bits to be the source address
	int firstByte = senderAddress << 6; 

	// Set second 2 bits to be destination address
	int destAddress = client->getMyAddr(); // TO-DO: implement dynamic destination address
	firstByte = firstByte | (destAddress << 4);
    // Create 2nd Header Byte
    int secondByte = 0b10000000;

    vector<char> char_vec;
    char_vec.push_back((char)firstByte);
    char_vec.push_back((char)secondByte);
    char_vec.insert(char_vec.end(),sendingTable.begin(),sendingTable.end());

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
	return output;
}

Message PacketGenerator::generateNextHopPacket(Message message, int destAddr){
    vector<char> data = message.data;
    int nextHop = findNextHop(destAddr);
    char secondByte = data[1];
    int leftThreeBits = secondByte & 0b11100000;
    int rightThreeBits = secondByte & 0b00000111;
    int newSecondByte = 0b00000000;
    newSecondByte = newSecondByte | leftThreeBits;
    newSecondByte = newSecondByte | rightThreeBits;
    newSecondByte = newSecondByte | (nextHop << 3);
    message.data[1] = newSecondByte;
    bitset<8> secondBytebit(newSecondByte);
    cout << "SEcondBYte of nexthop: " << secondBytebit << endl;
    return message;
};

vector<Message> PacketGenerator::generateNextHopPacketVec(Message message, int destAddr){
    Message newMessage = generateNextHopPacket(message, destAddr);
    vector<Message> output;
    output.push_back(newMessage);
    return output;
};

int PacketGenerator::findNextHop(int destAddr){
    int result = 99;
    const std::vector<std::vector<int>>& vec = *routingTable;
    for(int i = 0; i < 4; i++){
        if(vec[destAddr][i] < result){ // Choose smallest neighbour id
            result = i;
        }
    }
    if(result >= 4){
        printf("this node is unreachable!");
    }
    return result;
}