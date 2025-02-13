#include "Stun.h"
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

constexpr char stunServer[] = "stun.l.google.com";
constexpr short stunServerPort = 7001;

std::pair<std::string, short> GetPublicIp()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cout << "WSAStartup failed" << WSAGetLastError() << std::endl;
		return { "0.0.0.0", 0 };
	}

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		std::cout << "socket failed" << WSAGetLastError() << std::endl;
		WSACleanup();
		return { "0.0.0.0", 0 };
	}

	sockaddr_in stunServerAddr{};
	stunServerAddr.sin_family = AF_INET;
	inet_pton(AF_INET, stunServer, &stunServerAddr.sin_addr);
	stunServerAddr.sin_port = htons(stunServerPort);
	const char stunRequest[20] = 
	{
		0x00, 0x01, 0x00, 0x00,
		0x21, 0x12, 0xA4, 0x42,
		0x63, 0x29, 0x72, 0x6A, 
		0x7C, 0x58, 0x54, 0x6D,
		0x20, 0x6B, 0x75, 0x4F
	};

	if (sendto(sock, stunRequest, sizeof(stunRequest), 0, (sockaddr*)&stunServerAddr, sizeof(stunServerAddr)) == SOCKET_ERROR)
	{
		std::cout << "sendto failed" << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		return { "0.0.0.0", 0 };
	}

	char buffer[1024];
	sockaddr_in addr{};
	int clientAddrSize = sizeof(addr);
	
	int recvSize = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&addr, &clientAddrSize);
	if (recvSize == SOCKET_ERROR)
	{
		std::cout << "recvfrom failed" << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		return { "0.0.0.0", 0 };
	}

	char ipStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &buffer[28], ipStr, INET_ADDRSTRLEN);
	const short port = (buffer[26] << 8) | buffer[27];
	buffer[28] ^= 0x21;
	buffer[29] ^= 0x12;
	buffer[30] ^= 0xA4;
	buffer[31] ^= 0x42;

	closesocket(sock);
	WSACleanup();

	return { ipStr, port };
}