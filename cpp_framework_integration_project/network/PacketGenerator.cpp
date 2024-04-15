#include "PacketGenerator.hpp"

PacketGenerator::PacketGenerator(){}

vector<Message> PacketGenerator::generatePackets(std::string input, Client* client){
    PacketType packetType;
    if(input.size() > 30){
        packetType = MULTI;
    }
    else{
        packetType = SINGLE;
    }

    vector<Message> output;
    switch(packetType){
        case MULTI:
            output = generateMultiPacket(input, client);
            break;
        case SINGLE:
            output = generateSingleDataPacket(input, client);
            break;
    }
    return output;
}

vector<Message> PacketGenerator::generateAckPacket(int seqNum, Client* client, int destAddr){
    vector<Message> output;
    return output;
}

vector<Message> PacketGenerator::generateMultiPacket(std::string input, Client* client){
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

vector<Message> PacketGenerator::generateSingleDataPacket(std::string input, Client* client){
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