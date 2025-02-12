#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

constexpr short port = 9500;

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup failed" << WSAGetLastError() << std::endl;
        return 0;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        cout << "socket failed" << WSAGetLastError() << std::endl;
        WSACleanup();
        return 0;
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        cout << "bind failed" << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    sockaddr_in clientAddr{};
    int clientAddrSize = sizeof(clientAddr);
    char buffer[1024];

    while (true)
    {
        int recvSize = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientAddrSize);
        if (recvSize == SOCKET_ERROR)
        {
            cout << "recvfrom failed" << WSAGetLastError() << std::endl;
            break;
        }

        cout << "recvfrom: " << buffer << std::endl;
        if (sendto(sock, buffer, recvSize, 0, (sockaddr*)&clientAddr, clientAddrSize) == SOCKET_ERROR)
        {
            cout << "sendto failed" << WSAGetLastError() << std::endl;
            break;
        }
    }

    closesocket(sock);
    WSACleanup();

	return 0;
}