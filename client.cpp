// client.cpp
#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 44444;
const int BUFFER_SIZE = 4096;
const char *SERVER_IP = "127.0.0.1";

void receiveMessages(SOCKET serverSocket)
{
    char buffer[BUFFER_SIZE];
    while (true)
    {
        ZeroMemory(buffer, BUFFER_SIZE);
        int bytesReceived = recv(serverSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0)
        {
            std::cout << buffer;
        }
    }
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverHint;
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(PORT);
    serverHint.sin_addr.s_addr = inet_addr(SERVER_IP);

    connect(serverSocket, (sockaddr *)&serverHint, sizeof(serverHint));

    // Enter and send the username
    std::string name;
    std::cout << "Enter your name: ";
    std::getline(std::cin, name);
    send(serverSocket, name.c_str(), name.size() + 1, 0);

    // Start a thread to receive messages from the server
    std::thread receiveThread(receiveMessages, serverSocket);
    receiveThread.detach();

    // Send messages to the server
    std::string message;
    while (true)
    {
        std::getline(std::cin, message);
        if (!message.empty())
        {
            send(serverSocket, message.c_str(), message.size() + 1, 0);
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
