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

char send_buf[BUF_SIZE] = "hello!";
char recv_buf[BUF_SIZE];
struct sockaddr_in self_addr;
struct sockaddr_in bc_addr;
struct sockaddr_in recv_addr;
struct sockaddr_in sender_addr;
SOCKET bc_socket;
char s[BUF_SIZE];

void recvFunc(void* param) {
	int len = sizeof(struct sockaddr_in);
	while (true) {
		recvfrom(bc_socket, recv_buf, BUF_SIZE, 0, (sockaddr*)& recv_addr, &len);
		char* r = inet_ntoa(recv_addr.sin_addr);
		// ignoring messages from itself
		if (strcmp(r, s)) {
			Sleep(100);
			cout << "received message: " << recv_buf << " from " << r << endl;
		}
	}
}

void sendFunc(void* param) {
	while (true) {
		if (sendto(bc_socket, send_buf, strlen(send_buf) + 1, 0, (sockaddr*)& sender_addr, sizeof(sender_addr)) < 0) {
			perror("borhot send: ");
			_getch();
			closesocket(bc_socket);
		}
		cout << "broadcasted message: " << send_buf << endl;
		Sleep(3000);
	}
}


int main(void) {
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR) {
		cerr << "WSAStartup failed with error " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}

	// get self ip-address
	struct hostent* hp;
	char hostname[BUF_SIZE];
	gethostname(hostname, BUF_SIZE);
	hp = gethostbyname(hostname);
	memcpy(&self_addr.sin_addr, hp->h_addr_list[0], hp->h_length);
	strcpy_s(s, inet_ntoa(self_addr.sin_addr));

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

	sender_addr.sin_family = AF_INET;
	sender_addr.sin_port = htons(DEFAULT_PORT);
	sender_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

	if (bind(bc_socket, (sockaddr*)& bc_addr, sizeof(bc_addr)) == SOCKET_ERROR)	{
		perror("bind");
		_getch();
		closesocket(bc_socket);
		return 1;
	}

	_beginthread(recvFunc, 0, NULL);
	_beginthread(sendFunc, 0, NULL);
	
	cout << "spawned threads, press any key to exit.. \n";
	_getch();

	closesocket(bc_socket);
	WSACleanup();

	return 0;
}
