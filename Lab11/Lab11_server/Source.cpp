#pragma comment(lib, "Ws2_32.lib")
#include <winsock.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
using namespace std;

#define DEFAULT_PORT 5001
#define BUF_SIZE 128


class Registry {
	vector<string> reg;

public:
	Registry() { }
	Registry(string filename) {
		ifstream infile(filename);
		while (!infile.eof()) {
			string addr;
			infile >> addr;
			reg.push_back(addr);
		}
		infile.close();
	}

	const bool check_user(string addr) {
		cout << addr << endl;
		return find(reg.begin(), reg.end(), addr) != reg.end();
	}
};

SOCKET accept_client(SOCKET listen_socket, Registry & r) {
	struct sockaddr_in from;
	int fromlen = sizeof(from);
	SOCKET msgsock;

	while (true) {
		msgsock = accept(listen_socket, (struct sockaddr*) & from, &fromlen);
		if (msgsock == INVALID_SOCKET) {
			fprintf(stderr, "accept() error %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}
		string sin_addr = inet_ntoa(from.sin_addr);
		unsigned short sin_port = htons(from.sin_port);
		if (r.check_user(sin_addr)) {
			printf("accepted connection from %s, port %d\n",
				sin_addr.c_str(),
				sin_port);
			break;
		}
		closesocket(msgsock);
		printf("rejected connection from %s, port %d\n client not registered\n",
			sin_addr.c_str(),
			sin_port);
	}

	return msgsock;
}

class Dictionary {
	map<string, string> content;
	string filename;
	typedef pair<string, string> str_pair;

public:
	Dictionary() {	}
	Dictionary(string filename) {
		this->filename = filename;
		ifstream infile(filename);
		while (!infile.eof() && infile.peek() != ifstream::traits_type::eof()) {
			string pair;
			infile >> pair;
			content.insert(parse(pair));
		}
		infile.close();
	}

	str_pair parse(string s) {
		string delimiter = "#";
		size_t pos = s.find(delimiter);
		string key = s.substr(0, pos);
		string value = s.substr(pos+1);
		return str_pair(key, value);
	}

	string get(string key) {
		return content[key];
	}

	string execute(char Buffer[BUF_SIZE]) {

		string cmd = string(Buffer);
		size_t pos = cmd.find(' ');
		string pref = cmd.substr(0, pos);
		string key = cmd.substr(pos + 1);
		string res = "";
		if (pref == "add") {
			ofstream file(filename);
			file << key;
			content.insert(parse(key));
			file.close();
			cout << "Added " << key << " successfully\n";
		}
		else if (pref == "get") {
			res = content[key];
		}
		else {
			cout << "Invalid command '" << pref << "'\n";
		}
		return res;
	}
};



int main(void) {

	char Buffer[BUF_SIZE], c;
	unsigned short port = DEFAULT_PORT;
	int retval;
	int socket_type = SOCK_STREAM;   //TCP-protocol
	struct sockaddr_in local, from;
	WSADATA wsaData;
	SOCKET listen_socket, msgsock;
	Registry r("clients.txt");
	Dictionary d("dict.txt");

	if (WSAStartup(0x101, &wsaData) == SOCKET_ERROR) {
		cerr << "WSAStartup failed with error " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}

	//Create a socket
	listen_socket = socket(AF_INET, socket_type, 0); // TCP socket
	if (listen_socket == INVALID_SOCKET) {
		cerr << "Server: Error Opening socket: Error " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}
	//
	// bind() associates a local address and port combination with the
	// socket just created.
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(port);  // Port MUST be in Network Byte Order

	if (bind(listen_socket, (struct sockaddr*) & local, sizeof(local))
		== SOCKET_ERROR) {
		cerr << "bind() failed: error " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}

	if (listen(listen_socket, 5) == SOCKET_ERROR) {
		cerr << "listen() failed: error " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}
	cout << "Server listening on port " << port << " , protocol TCP\n";

	while (true) {

		msgsock = accept_client(listen_socket, r);

		string buf = "Server waiting for commands";
		retval = send(msgsock, buf.c_str(), buf.size() + 1, 0);
		if (retval == SOCKET_ERROR) {
			cerr << "Server: send() failed: error " << WSAGetLastError() << endl;
		}

		while (true) {

			retval = recv(msgsock, Buffer, sizeof(Buffer), 0);
			if (retval == SOCKET_ERROR) {
				cerr << "Server: recv() failed: error " << WSAGetLastError() << endl;
				closesocket(msgsock);
				return -1;
			}
			if (retval == 0) {
				cout << "Client closed connection\n";
				closesocket(msgsock);
			}
			cout << "Received " << retval << " bytes, data [" << Buffer << "] from client\n";

			
			// Parse & execute command and send result to client
			buf = d.execute(Buffer);
			retval = send(msgsock, buf.c_str(), buf.size() + 1, 0);
			if (retval == SOCKET_ERROR) {
				cerr << "Server: send() failed: error " << WSAGetLastError() << endl;
			}

		}
	}

	closesocket(msgsock);
	WSACleanup();
	return 0;
}


