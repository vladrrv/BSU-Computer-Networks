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
	SOCKET  conn_socket;

	socket_type = SOCK_STREAM;   // TCP-protocol

	if (WSAStartup(0x101, &wsaData) == SOCKET_ERROR) {
		fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	fprintf(stdout, "Input server IP-address\n");
	getline(cin, server_name);

	if (isalpha(server_name[0])) {   /* server address is a name */
		hp = gethostbyname(server_name.c_str());
	}
	else { /* Convert nnn.nnn address to a usable one */
		addr = inet_addr(server_name.c_str());
		hp = gethostbyaddr((char*)& addr, 4, AF_INET);
	}
	if (hp == NULL) {
		fprintf(stderr, "Client: Cannot resolve address [%s]: Error %d\n",
			server_name.c_str(), WSAGetLastError());
		WSACleanup();
		exit(1);
	}
	// Create a socket
	conn_socket = socket(AF_INET, socket_type, 0);
	if (conn_socket == INVALID_SOCKET) {
		fprintf(stderr, "Client: Error Opening socket: Error %d\n",
			WSAGetLastError());
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
		fprintf(stderr, "connect() failed: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	retval = recv(conn_socket, Buffer, sizeof(Buffer), 0);
	if (retval == SOCKET_ERROR) {
		fprintf(stderr, "recv() failed: error %d\n", WSAGetLastError());
		closesocket(conn_socket);
		WSACleanup();
		return -1;
	}
	if (retval == 0) {
		printf("Server closed connection\n");
		closesocket(conn_socket);
		WSACleanup();
		return -1;
	}
	printf("Received %d bytes, data [%s] from server\n", retval, Buffer);


	while (c != 'e') {

		fprintf(stdout, "Input message\n");
		string buf;
		getline(cin, buf);

		retval = send(conn_socket, buf.c_str(), buf.size() + 1, 0);
		if (retval == SOCKET_ERROR) {
			fprintf(stderr, "send() failed: error %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}
		printf("Sent Data [%s]\n", buf.c_str());
		retval = recv(conn_socket, Buffer, sizeof(Buffer), 0);
		if (retval == SOCKET_ERROR) {
			fprintf(stderr, "recv() failed: error %d\n", WSAGetLastError());
			closesocket(conn_socket);
			WSACleanup();
			return -1;
		}

		if (retval == 0) {
			printf("Server closed connection\n");
			closesocket(conn_socket);
			WSACleanup();
			return -1;
		}
		printf("Received %d bytes, data [%s] from server\n", retval, Buffer);

		printf("Press Enter to continue, 'e' to exit");
		c = getc(stdin);
	}
	closesocket(conn_socket);
	WSACleanup();

	return 0;
}
