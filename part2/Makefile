# the compiler: gcc for C program, define as g++ for C++
CC = g++

# compiler flags:
CFLAGS  = -std=c++11 -Wall -pthread

TARGET = main
PORT = 35808
HOSTFILE = hostnames.txt
COUNT = 10
X = 9
DELAY = 2
LOG = info

LIB_FILES = ../lib/MessageQueue.cpp ../lib/Message.cpp ../lib/Log.cpp ../lib/LogHelper.cpp ../lib/helpers.cpp ../lib/DataMessageSeqTracker.cpp ../lib/ListenerSocket.cpp ../lib/SeqProvider.cpp ../lib/SocketImpl.cpp ../lib/MessageDispatcher.cpp ../lib/SenderSocket.cpp ../lib/ProcessInfoHelper.cpp ../lib/MessageHandler.cpp ../lib/DeliveryTracker.cpp ../lib/NetworkStatus.cpp ../lib/HandshakeTracker.cpp ../lib/TcpListener.cpp ../lib/TcpSender.cpp ../lib/SnapshotHandler.cpp

buildccs: cleanccs build

build: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) $(LIB_FILES) $(TARGET).cpp -o $(TARGET)

run:
	./$(TARGET) -p $(PORT) -h $(HOSTFILE) -c $(COUNT) -v $(LOG) -d $(DELAY) -l -x $(X)

runsnapshot:
	./$(TARGET) -p $(PORT) -h $(HOSTFILE) -c $(COUNT) -x $(X)

generate:
	echo "vdi-linux-030.ccs.neu.edu" >> hostnames.txt
	echo "vdi-linux-031.ccs.neu.edu" >> hostnames.txt
	echo "vdi-linux-032.ccs.neu.edu" >> hostnames.txt
	echo "vdi-linux-033.ccs.neu.edu" >> hostnames.txt
	echo "vdi-linux-034.ccs.neu.edu" >> hostnames.txt

clean:
	rm -f $(TARGET)
	rm -f *.txt
