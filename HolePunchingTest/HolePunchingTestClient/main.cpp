#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

constexpr short port = 9500;
constexpr const char* serverIp = "127.0.0.1";

bool TryHolepunching(SOCKET sock, sockaddr_in& serverAddr, const int timeoutMs, const int maxPuncherCount)
{
	for (int i = 0; i < maxPuncherCount; ++i)
	{
		if (sendto(sock, "Hello", 5, 0, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			std::cout << "sendto failed" << WSAGetLastError() << std::endl;
			return false;
		}
	}

	fd_set fdSet;
	FD_ZERO(&fdSet);
	FD_SET(sock, &fdSet);

	timeval timeout = {};
	timeout.tv_sec = timeoutMs / 1000;
	timeout.tv_usec = (timeoutMs % 1000) * 1000;

	int retval = select(0, &fdSet, nullptr, nullptr, &timeout);
	if (retval == SOCKET_ERROR)
	{
		std::cout << "select() failed with " << WSAGetLastError() << std::endl;
		return false;
	}

	return retval > 0;
}

void ClientMainLoop(SOCKET sock)
{
	constexpr int bufferSize = 1024;
	char buffer[bufferSize];
	sockaddr_in clientAddr = {};
	int clientAddrSize = sizeof(clientAddr);

	while (true)
	{
		int recvSize = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientAddrSize);
		if (recvSize == SOCKET_ERROR)
		{
			std::cout << "recvfrom failed" << WSAGetLastError() << std::endl;
			break;
		}

		if (bufferSize <= recvSize)
		{
			std::cout << "buffer is over with recv size " << recvSize << std::endl;
			break;
		}

		buffer[recvSize] = '\0';
		std::cout << "recvfrom: " << buffer << std::endl;
	}
}

int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cout << "WSAStartup failed " << WSAGetLastError() << std::endl;
		return 0;
	}

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		std::cout << "socket failed" << WSAGetLastError() << std::endl;
		WSACleanup();
		return 0;
	}

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(0);

	if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		std::cout << "bind failed" << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		return 0;
	}
	std::cout << "Client started" << std::endl;

	constexpr int maxPuncherCount = 10;
	constexpr int timeoutMs = 1000;
	if (TryHolepunching(sock, serverAddr, timeoutMs, maxPuncherCount))
	{
		std::cout << "Holepunching success" << std::endl;

		ClientMainLoop(sock);
	}
	else
	{
		std::cout << "Holepunching failed" << std::endl;
		std::cout << "Use TURN server relay" << std::endl;

		ClientMainLoop(sock);
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}