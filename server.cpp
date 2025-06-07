#include <iostream>
#include <string>
#include <unordered_map>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 44444;
const int BUFFER_SIZE = 4096;

// map to store each client's socket and their username
std::unordered_map<SOCKET, std::string> clients;

void broadcastMessage(const std::string &message, SOCKET senderSocket)
{
    for (const auto &client : clients)
    {
        SOCKET clientSocket = client.first;
        const std::string &username = client.second;
        if (clientSocket != senderSocket)
        {
            send(clientSocket, message.c_str(), message.size() + 1, 0);
        }
    }
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverHint;
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(PORT);
    serverHint.sin_addr.s_addr = INADDR_ANY;

    bind(listeningSocket, (sockaddr *)&serverHint, sizeof(serverHint));
    listen(listeningSocket, SOMAXCONN);

    fd_set masterSet;
    FD_ZERO(&masterSet);
    FD_SET(listeningSocket, &masterSet);

    std::cout << "Server started on port " << PORT << ". Waiting for clients...\n";

    while (true)
    {
        fd_set copySet = masterSet;
        int socketCount = select(0, &copySet, nullptr, nullptr, nullptr);

        for (int i = 0; i < socketCount; i++)
        {
            SOCKET currentSocket = copySet.fd_array[i];

            if (currentSocket == listeningSocket)
            {
                // New client connection
                SOCKET clientSocket = accept(listeningSocket, nullptr, nullptr);
                FD_SET(clientSocket, &masterSet);

                // Send welcome message
                char welcomeMsg[] = "Welcome!\n";
                send(clientSocket, welcomeMsg, sizeof(welcomeMsg), 0);
                clients[clientSocket] = ""; // Placeholder until they send their name
            }
            else
            {
                // Receiving data from an existing client
                char buffer[BUFFER_SIZE];
                ZeroMemory(buffer, BUFFER_SIZE);
                int bytesReceived = recv(currentSocket, buffer, BUFFER_SIZE, 0);

                if (bytesReceived <= 0)
                {
                    // Client disconnected
                    std::cout << clients[currentSocket] << " disconnected.\n";
                    closesocket(currentSocket);
                    FD_CLR(currentSocket, &masterSet);
                    clients.erase(currentSocket);
                }
                else
                {
                    std::string message = buffer;

                    // Check if we have the client's name
                    if (clients[currentSocket].empty())
                    {
                        clients[currentSocket] = message;
                        std::string welcome = "[" + message + "] joined the chat.\n";
                        std::cout << welcome;
                        broadcastMessage(welcome, currentSocket);
                    }
                    else
                    {
                        // Broadcast the message with the client's name
                        std::string fullMessage = "[" + clients[currentSocket] + "] " + message + "\n";
                        std::cout << fullMessage;
                        broadcastMessage(fullMessage, currentSocket);
                    }
                }
            }
        }
    }

    WSACleanup();
    return 0;
}
