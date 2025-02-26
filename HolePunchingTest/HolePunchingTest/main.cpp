#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

constexpr short port = 9500;

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "WSAStartup failed" << WSAGetLastError() << std::endl;
        return 0;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cout << "socket failed" << WSAGetLastError() << std::endl;
        WSACleanup();
        return 0;
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::cout << "bind failed" << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    constexpr int bufferSize = 1024;
    char buffer[bufferSize];
    sockaddr_in clientAddr{};
    int clientAddrSize = sizeof(clientAddr);

	std::cout << "Server started" << std::endl;

    while (true)
    {
        int recvSize = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientAddrSize);
        if (recvSize == SOCKET_ERROR)
        {
            std::cout << "recvfrom failed" << WSAGetLastError() << std::endl;
            break;
        }

        if (recvSize >= bufferSize)
        {
			std::cout << "Recv size is too big " << recvSize << std::endl;
			continue;
        }

        buffer[recvSize] = '\0';

        std::cout << "recvfrom: " << buffer << std::endl;
        if (sendto(sock, buffer, recvSize, 0, (sockaddr*)&clientAddr, clientAddrSize) == SOCKET_ERROR)
        {
            std::cout << "sendto failed" << WSAGetLastError() << std::endl;
            break;
        }
    }

    closesocket(sock);
    WSACleanup();

	return 0;
}