#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

constexpr const unsigned short port = 50001;
constexpr const int bufferSize = 1024;

std::string GetClientKey(const std::unordered_map<std::string, sockaddr_in>& clients, const sockaddr_in& clientAddr)
{
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);
	return std::string(ip) + ":" + std::to_string(ntohs(clientAddr.sin_port));
}

enum class MessageType : char
{
	Error = 0,
	Alloc,
	Allocated,
	Send,
};

int main()
{
	std::unordered_map<std::string, sockaddr_in> clients;

	WSADATA wsaData;
	SOCKET serverSocket;
	sockaddr_in serverAddr, clientAddr;
	int addrLen = sizeof(clientAddr);
	char buffer[bufferSize];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed with " << GetLastError() << std::endl;
        return 0;
    }

	serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (serverSocket == INVALID_SOCKET) 
	{
		std::cerr << "Socket create failed with " << GetLastError() << std::endl;
		WSACleanup();
		return 0;
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Bind failed with " << GetLastError() << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}

	std::cout << "TURN server start" << std::endl;

	while (true)
	{
		memset(buffer, 0, bufferSize);
		int recvLength = recvfrom(serverSocket, buffer, bufferSize, 0, (sockaddr*)&clientAddr, &addrLen);
		if (recvLength == SOCKET_ERROR) 
		{
			std::cerr << "Recvfrom failed with " << GetLastError() << std::endl;
			continue;
		}
		else if (recvLength < 1)
		{
			std::cerr << "Invalid recv size" << std::endl;
			continue;
		}

		MessageType msgType = static_cast<MessageType>(buffer[0]);
		std::string clientKey = GetClientKey(clients, clientAddr);

		switch (msgType)
		{
		case MessageType::Alloc:
		{
			clients[clientKey] = { clientAddr };
			std::cout << "Allocated relay for " << clientKey << std::endl;

			char response = static_cast<char>(MessageType::Allocated);
			if (sendto(serverSocket, &response, 1, 0, (sockaddr*)&clientAddr, addrLen) == SOCKET_ERROR)
			{
				std::cout << "Sendto failed with " << GetLastError() << std::endl;
			}
			break;
		}
		case MessageType::Send:
		{
			std::cout << "Message from " << clientKey << std::endl;

			for (auto const& client : clients)
			{
				if (client.first != clientKey)
				{
					if (sendto(serverSocket, buffer, recvLength, 0, (sockaddr*)&client.second.sin_addr, addrLen) == SOCKET_ERROR)
					{
						std::cout << "Sendto failed with " << GetLastError() << std::endl;
					}
					else
					{
						std::cout << "Message relayed to " << client.first << std::endl;
					}
				}
			}
			break;
		}
		default:
		{
			std::cout << "Unknown message type from " << clientKey << " with message type " << static_cast<char>(msgType) << std::endl;
			break;
		}
		}
	}

	closesocket(serverSocket);
	WSACleanup();

	return 0;
}