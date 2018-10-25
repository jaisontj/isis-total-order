# the compiler: gcc for C program, define as g++ for C++
CC = g++

# compiler flags:
CFLAGS  = -std=c++11 -Wall -pthread

OUTPUT = prj1_tm
TARGET = main
PORT = 35808
HOSTFILE = hostnames.txt
COUNT = 10

LIB_FILES = ../lib/MessageQueue.cpp ../lib/Message.cpp ../lib/Log.cpp ../lib/LogHelper.cpp ../lib/helpers.cpp ../lib/DataMessageSeqTracker.cpp ../lib/ListenerSocket.cpp ../lib/SeqProvider.cpp ../lib/SocketImpl.cpp ../lib/MessageDispatcher.cpp ../lib/SenderSocket.cpp ../lib/ProcessInfoHelper.cpp ../lib/MessageHandler.cpp ../lib/DeliveryTracker.cpp ../lib/NetworkStatus.cpp

build: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) $(LIB_FILES) $(TARGET).cpp -o $(OUTPUT)

run:
	./$(OUTPUT) -p $(PORT) -h $(HOSTFILE) -c $(COUNT)

clean:
	rm -f $(OUTPUT)
	rm -f *.txt

generate:
	echo "vdi-linux-030.ccs.neu.edu" >> hostnames.txt
	echo "vdi-linux-031.ccs.neu.edu" >> hostnames.txt
	echo "vdi-linux-032.ccs.neu.edu" >> hostnames.txt
	echo "vdi-linux-033.ccs.neu.edu" >> hostnames.txt
	echo "vdi-linux-034.ccs.neu.edu" >> hostnames.txt
