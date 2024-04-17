#include <iostream>
#include <string>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#elif __linux__
#include <sys/socket.h>    	//socket
#include <arpa/inet.h> 		//inet_addr
#endif

#include "TUI.h"

using namespace std;

TUI::TUI(Client* client){
    this->client = client;
}

int TUI::setDestinationAddress(){
    bool input_valid = false;
    string destAddress;

    //wrongly initialized, so it has to change.
    char dest_addr = '4';
    printf("You can set the destination address to the following nodes: ");
    //print statement with possibilities, excluding their own.

    //Loop until valid input
    while(!input_valid){
        getline(cin, destAddress);
        dest_addr = destAddress.at(0);
        my_addr = client->getMyAddr();
        if(dest_addr != my_addr && (my_addr == '0' || my_addr == '1' || my_addr == '2' || my_addr == '3')){
			input_valid = true;
        } else {
            cout << "Invalid input, please enter 0, 1, 2 or 3." << destAddress <<endl;
        }
    }
    return dest_addr - '0';
}