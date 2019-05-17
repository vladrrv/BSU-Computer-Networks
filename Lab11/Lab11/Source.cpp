#pragma comment(lib, "Ws2_32.lib")
#include <winsock.h>
#include <string>
#include <iostream>
using namespace std;

#define DEFAULT_PORT 5001

int main(void) {

	char Buffer[128], c = 0;
	string server_name;
	unsigned short port = DEFAULT_PORT;
	int retval;
	unsigned int addr;
	int socket_type;
	struct sockaddr_in server;
	struct hostent* hp;
	WSADATA wsaData;
	SOCKET conn_socket;

	socket_type = SOCK_STREAM;   // TCP-protocol

	if (WSAStartup(0x101, &wsaData) == SOCKET_ERROR) {
		cerr << "WSAStartup failed with error " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}

	cout << "Input server IP-address\n";
	cin >> server_name;
	cin.get();

	if (isalpha(server_name[0])) {   /* server address is a name */
		hp = gethostbyname(server_name.c_str());
	}
	else { /* Convert nnn.nnn address to a usable one */
		addr = inet_addr(server_name.c_str());
		hp = gethostbyaddr((char*)& addr, 4, AF_INET);
	}
	if (hp == NULL) {
		cerr << "Client: Cannot resolve address [" << server_name << "]: Error " 
			<< WSAGetLastError() << endl;
		WSACleanup();
		exit(1);
	}
	// Create a socket
	conn_socket = socket(AF_INET, socket_type, 0);
	if (conn_socket == INVALID_SOCKET) {
		cerr << "Client: Error Opening socket: Error " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}

	// Copy into the sockaddr_in structure

	memset(&server, 0, sizeof(server));
	memcpy(&(server.sin_addr), hp->h_addr, hp->h_length);
	server.sin_family = hp->h_addrtype;
	server.sin_port = htons(port);

	printf("Client connecting to: %s\n", hp->h_name);
	if (connect(conn_socket, (struct sockaddr*) & server, sizeof(server))
		== SOCKET_ERROR) {
		cerr << "connect() failed: " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}

	retval = recv(conn_socket, Buffer, sizeof(Buffer), 0);
	if (retval == SOCKET_ERROR) {
		cerr << "recv() failed: error " << WSAGetLastError() << endl;
		closesocket(conn_socket);
		WSACleanup();
		return -1;
	}
	if (retval == 0) {
		cout << "Server closed connection\n";
		closesocket(conn_socket);
		WSACleanup();
		return -1;
	}
	cout << "Received " << retval << " bytes, data [" << Buffer << "] from server\n";


	while (c != 27) {

		cout << "Input message\n";
		string buf;
		getline(cin, buf);

		retval = send(conn_socket, buf.c_str(), buf.size() + 1, 0);
		if (retval == SOCKET_ERROR) {
			cerr << "send() failed: error " << WSAGetLastError() << endl;
			WSACleanup();
			return -1;
		}
		//cout << "Sent Data [" << buf <<  "]\n";
		retval = recv(conn_socket, Buffer, sizeof(Buffer), 0);
		if (retval == SOCKET_ERROR) {
			cerr << "recv() failed: error " << WSAGetLastError() << endl;
			closesocket(conn_socket);
			WSACleanup();
			return -1;
		}

		if (retval == 0) {
			cout << "Server closed connection\n";
			closesocket(conn_socket);
			WSACleanup();
			return -1;
		}
		//cout << "Received " << retval << " bytes, data [" << Buffer << "] from server\n";
		cout << "Received '" << Buffer << "'\n";
		cout << "Press Enter to continue, Esc to exit\n";
		c = cin.get();
	}
	closesocket(conn_socket);
	WSACleanup();

	return 0;
}
