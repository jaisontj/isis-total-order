#include<iostream>
#include<unistd.h>
#include<fstream>
#include<string>
#include<vector>
using namespace std;

static bool DEBUG = true;

void debugPrint(string message) {
	if (DEBUG)
		cout<<message<<endl;
}

void showUsageAndExit() {
	cout<<"Usage: -p port -h hostfile -c count"<<endl;
	exit(0);
}

void checkAndParseArgs(int argc, char* argv[], int &port, int &count, string &hostfile) {
	int opt;
	while((opt = getopt(argc, argv, "p:h:c:")) != -1) {
		switch(opt) {
			case 'p':
				port = atoi(optarg);
				break;
			case 'h':
				hostfile = optarg;
				break;
			case 'c':
				count = atoi(optarg);
				break;
			default: 
				showUsageAndExit();
				break;
		}
	}

	if(port == -1 || count == -1 || hostfile == "")
		showUsageAndExit();

	debugPrint("Port: " + to_string(port));
	debugPrint("Count: " + to_string(count));
	debugPrint("Hostfile: " + hostfile);
}

vector<string> getHostnamesFromFile(string hostfile) {
	string line;
	vector<string> hostnames;
	ifstream file (hostfile);
	if (file.is_open()) {
		debugPrint("\nOpened file at " + hostfile + " successfully");
		debugPrint("Printing hostnames: ");
		while(getline(file, line)) {
			debugPrint("\t" + line);
			hostnames.push_back(line);
		}
		file.close();
	} else {
		cout<<"Unable to open file: "<<hostfile<<endl;
		exit(0);
	}
	return hostnames;
}



int main(int argc, char* argv[]){
	int port = -1, count = -1;
	string hostfile = "";
	checkAndParseArgs(argc, argv, port, count, hostfile);

	//Read hostfile for a list of hosts.
	vector<string> hostnames = getHostnamesFromFile(hostfile);
	
}
