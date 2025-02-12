#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

constexpr short port = 9500;
constexpr const char* serverIp = "127.0.0.1";

int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup failed " << WSAGetLastError() << std::endl;
		return 0;
	}

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cout << "socket failed" << WSAGetLastError() << std::endl;
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
		cout << "bind failed" << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		return 0;
	}
	
	constexpr int bufferSize = 1024;
	char buffer[bufferSize];
	sockaddr_in clientAddr = {};
	int clientAddrSize = sizeof(clientAddr);

	constexpr int maxPuncherCount = 10;
	for (int i = 0; i < maxPuncherCount; ++i)
	{
		if (sendto(sock, "Hello", 5, 0, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			cout << "sendto failed" << WSAGetLastError() << std::endl;
			break;
		}
	}

	while (true)
	{
		int recvSize = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientAddrSize);
		if (recvSize == SOCKET_ERROR)
		{
			cout << "recvfrom failed" << WSAGetLastError() << std::endl;
			break;
		}

		if (bufferSize <= recvSize)
		{
			cout << "buffer is over with recv size " << recvSize << std::endl;
			break;
		}

		buffer[recvSize] = '\0';
		cout << "recvfrom: " << buffer << std::endl;
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}