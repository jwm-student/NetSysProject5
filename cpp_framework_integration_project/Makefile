 # makefile for use on Linux; may also work on other operating systems...
.SILENT:

src_files := $(shell find -type f -name '*.cpp')
LDFLAGS		= -lpthread

OBJS = network/Client.cpp network/DataReceiver.cpp main.cpp

integration: $(src_files)
	g++ $(LDFLAGS) $(OBJS) -o integration

clean:
	rm integration


#To run the following program, compile it with the following command:
# g++ -lpthread network/CollisionAvoidance.cpp network/Client.cpp network/DataReceiver.cpp network/PacketGenerator.cpp network/PacketProcessor.cpp network/DVR.cpp utils/TUI.cpp  main.cpp -lws2_32 -o integration
#./integration