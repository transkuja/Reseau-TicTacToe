#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <thread>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string>
#define DEFAULT_PORT "52250"

void ExchangeWithClientLoop(SOCKET ClientSocket, int _idPlayer)
{
	int instructionsResult;
	
	char recvbuf[10];
	int sendResult;
	int recvbuflen = 10;
	int idPlayer = _idPlayer;

	// Receive until the peer shuts down the connection
	do {
		// recv function returns the number of bytes received or an error
		instructionsResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

		// Bytes received => process
		if (instructionsResult > 0) {
			printf("Msg received: %s\n", recvbuf);
			if (recvbuf.find())
			sendResult = send(ClientSocket, recvbuf, instructionsResult, 0); 
			printf("Msg sent: %s\n", recvbuf);

		}
		else if (instructionsResult == 0)
			printf("Connection closing...\n");

		// Error case
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return;
		}

		// Repeat while data are received
	} while (instructionsResult > 0);

}

int main() {
	int instructionsResult;
	WSADATA wsaData;
	struct addrinfo *result = NULL;
	struct addrinfo	hints;

	// Used to listen for client connections
	SOCKET ListenSocket = INVALID_SOCKET;

	// Used for accepting client connections (multithread required to listen for connections from multiple clients)
	SOCKET ClientSocket;

	std::thread threads[2];

	// Initialize Winsock
	instructionsResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (instructionsResult != 0) {
		printf("WSAStartup failed with error: %d\n", instructionsResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the local address and port to be used by the server
	instructionsResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (instructionsResult != 0) {
		printf("getaddrinfo failed: %d\n", instructionsResult);
		WSACleanup();
		return 1;
	}

	// Create the socket to listen to for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);


	// Bind the socket with informations retrieved by getaddrinfo
	instructionsResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);

	// Security check
	if (instructionsResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Free the address information returned above cause we no longer need it past the binding step
	freeaddrinfo(result);

	// Start listening with ListenSocket, accepting a maximum of SOMAXCONN pending connections
	if (listen(ListenSocket, 2) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	bool isRunning = true;
	int nbrOfThreadActive = 0;

	while (nbrOfThreadActive < 2)
	{
		ClientSocket = INVALID_SOCKET; // Initialize value
		// Accept a client socket (multithread required to listen to multiple clients)
		ClientSocket = accept(ListenSocket, NULL, NULL);

		// Security checks on client socket
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		threads[nbrOfThreadActive] = std::thread(ExchangeWithClientLoop, ClientSocket, nbrOfThreadActive);
		std::string sendBuff = std::to_string(nbrOfThreadActive);
		send(ClientSocket, sendBuff.c_str(), instructionsResult, 0); // send the player its id

		nbrOfThreadActive++;
	}

	for (int i = 0; i < nbrOfThreadActive; i++)
	{
		threads[i].join();
	}

	
	// Shutdown the connection since we no longer receive/send anything
	instructionsResult = shutdown(ClientSocket, SD_SEND);
	if (instructionsResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// Cleanup
	closesocket(ClientSocket); // close the client socket
	WSACleanup(); // Stop using the dll

	return 0;
}