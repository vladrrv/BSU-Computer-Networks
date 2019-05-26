#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#include <winsock2.h>
#include <string>
#include <iostream>
#include <conio.h>
#include <process.h>
using namespace std;

#define DEFAULT_PORT 5001

#define BUF_SIZE 128

char send_buf[BUF_SIZE] = "hi!";
char recv_buf[BUF_SIZE];
struct sockaddr_in bc_addr;
struct sockaddr_in recv_addr;
SOCKET bc_socket;

void recvFunc() {
	cout << "waiting for broadcast message..." << endl;

	int len = sizeof(struct sockaddr_in);
	recvfrom(bc_socket, recv_buf, BUF_SIZE, 0, (sockaddr*)& recv_addr, &len);
	cout << "received message: " << recv_buf << " from " << inet_ntoa(recv_addr.sin_addr) << endl;

	if (sendto(bc_socket, send_buf, strlen(send_buf) + 1, 0, (sockaddr*)& recv_addr, sizeof(recv_addr)) < 0) {
		perror("borhot send: ");
		_getch();
		closesocket(bc_socket);
	}
	cout << "sent message: " << send_buf << " to " << inet_ntoa(recv_addr.sin_addr) << endl;
}


int main(void) {
	WSADATA wsaData;
	struct hostent* hp;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR) {
		cerr << "WSAStartup failed with error " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}

	bc_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	char broadcast = 'a';
	if (setsockopt(bc_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, 1) == SOCKET_ERROR) {
		perror("broadcast options");
		_getch();
		closesocket(bc_socket);
		return 1;
	}

	bc_addr.sin_family = AF_INET;
	bc_addr.sin_port = htons(DEFAULT_PORT);
	bc_addr.sin_addr.s_addr = INADDR_ANY;

	recv_addr.sin_family = AF_INET;
	recv_addr.sin_port = htons(DEFAULT_PORT);
	recv_addr.sin_addr.s_addr = INADDR_ANY;


	if (bind(bc_socket, (sockaddr*)& bc_addr, sizeof(bc_addr)) == SOCKET_ERROR) {
		perror("bind");
		_getch();
		closesocket(bc_socket);
		return 1;
	}

	recvFunc();

	cout << "press any key to exit... \n";
	_getch();

	closesocket(bc_socket);
	WSACleanup();

	return 0;
}
