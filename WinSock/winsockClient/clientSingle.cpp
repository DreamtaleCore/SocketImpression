#include <winsock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


const int defaultBuffLen = 512;
const char* defaultPort = "27015";
string serverAddr = "192.168.1.104";

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET connectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	string sendBuf = "this is a test";
	char recvBuf[defaultBuffLen];
	int iResult;
	int recvBufLen = defaultBuffLen;

	// Copy the server parameters
	if (argc > 1) 
	{
		serverAddr = argv[1];
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) 
	{
		cout << "WSAStartup failed with error: " << iResult << endl;
		return 1;
	}

	// Init the hints
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(serverAddr.c_str(), defaultPort, &hints, &result);
	if (iResult != 0) 
	{
		cout << "getaddrinfo failed with error: " << iResult << endl;
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) 
	{
		// Create a SOCKET for connecting to server
		connectSocket = socket(
			ptr->ai_family,
			ptr->ai_socktype,
			ptr->ai_protocol);
		if (connectSocket == INVALID_SOCKET) 
		{
			cout << "socket failed with error: " << WSAGetLastError() << endl;
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) 
		{
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (connectSocket == INVALID_SOCKET) 
	{
		cout << "Unable to connect to server :" << endl 
			 << "\t" << serverAddr << ":" << defaultPort << endl;
		WSACleanup();
		return 1;
	}

	// Send an initial buffer
	cout << "Connect the server(" << serverAddr << ") successfully!" << endl;
	cout << "******************************************************" << endl;
	cout << "Say something:" << endl;
	do
	{
		cin >> sendBuf;
		iResult = send(connectSocket, sendBuf.c_str(), sendBuf.length(), 0);
		if (iResult == SOCKET_ERROR) {
			cout << "send failed with error: " << WSAGetLastError() << endl;
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}

		cout << "Bytes Sent: " << iResult << endl;

		// Receive until the peer closes the connection
		memset(recvBuf, '\0', recvBufLen);
		iResult = recv(connectSocket, recvBuf, recvBufLen, 0);
		if (iResult > 0)
		{
			if (recvBuf[0] == '\0')
				break;
			cout << recvBuf << endl;
			cout << "Bytes received: " << iResult << endl;
		}
		else if (iResult == 0)
			cout << "Connection closed." << endl;
		else
			cout << "recv failed with error: " << WSAGetLastError() << endl;
		cout << "-------------------------" << endl;

	} while (sendBuf.compare("exit"));
	// cleanup
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}