#ifndef CLIENT_HPP
#define CLIENT_HPP
#include "../utils/Message.h"

class Client {

private:
	string server_addr;
	int server_port;
	int frequency;
	string token;
	BlockingQueue< Message > *senderQueue;
	BlockingQueue< Message > *receiverQueue;
	Message TypeMessage;
	int seqNum;
	int expSeqNum;
	char myAddr;

	int openSocket();

public:
	Client(string server_addr, char myAddr, int server_port,  int frequency, string token, BlockingQueue< Message > *senderQueue, BlockingQueue< Message > *receiverQueue);

	void startThread();
	void listener(int sock);
	void setMyAddr(char newAddr);
	char getMyAddr();
	void readInput(BlockingQueue< Message >*senderQueue, char addr);
	int getSeqNum();
	void setSeqNum(int seqNum);
	void increaseSeqNum();
	int getExpSeqNum();
	void setExpSeqNum(int expSeqNum);
	void increaseExpSeqNum();
};

#endif