# the compiler: gcc for C program, define as g++ for C++
CC = g++

# compiler flags:
CFLAGS  = -std=c++11 -Wall -pthread

TARGET = main
PORT = 35808
HOSTFILE = hostnames.txt
COUNT = 10

LIB_FILES = ../lib/helpers.cpp ../lib/socket_helpers.cpp ../lib/DataMessageQueue.cpp ../lib/ProposalTracker.cpp ../lib/SeqProvider.cpp

all: clean build run

alldebug: clean build rundebug

build: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) $(LIB_FILES) $(TARGET).cpp -o $(TARGET)

run:
	./$(TARGET) -p $(PORT) -h $(HOSTFILE) -c $(COUNT)

rundebug:
	./$(TARGET) -p $(PORT) -h $(HOSTFILE) -c $(COUNT) -d


clean:
	rm -r $(TARGET)
