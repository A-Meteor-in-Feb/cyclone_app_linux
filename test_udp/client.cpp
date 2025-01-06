#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

#define SERVER_IP "192.168.0.167" 
#define PORT 8080
#define BUFFER_SIZE 1024


int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Socket creation failed." << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported." << std::endl;
        close(clientSocket);
        return 1;
    }


    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed." << std::endl;
        close(clientSocket);
        return 1;
    }


    std::cout << "Connected to server!" << std::endl;
    char buffer[BUFFER_SIZE];
    while (true) {
        std::cout << "Enter message: ";
        std::cin.getline(buffer, BUFFER_SIZE);
        send(clientSocket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            std::cout << "Connection closed by server." << std::endl;
            break;
        }
        std::cout << "Server: " << buffer << std::endl;

    }

    close(clientSocket);
    return 0;

}