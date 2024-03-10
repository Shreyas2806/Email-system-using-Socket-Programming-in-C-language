#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main() {
    // Initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        perror("WSAStartup failed");
        exit(EXIT_FAILURE);
    }

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE] = "Welcome to the server!";

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        perror("Socket creation failed");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind the socket
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Binding failed");
        closesocket(server_sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_sock, 5) == SOCKET_ERROR) {
        perror("Listening failed");
        closesocket(server_sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", SERVER_PORT);

    while (1) {
        // Accept incoming connection
        int client_addr_len = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_sock == INVALID_SOCKET) {
            perror("Acceptance failed");
            closesocket(server_sock);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // Send a welcome message to the client
        send(client_sock, buffer, strlen(buffer), 0);

        // Receive and display messages from the client
        while (1) {
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
                perror("Connection closed");
                break;
            }

            printf("[*] Client Message: %s", buffer);

            // Send a response back to the client
            send(client_sock, "Message received by the server.\n", 32, 0);
        }

        // Close the client socket
        closesocket(client_sock);
    }

    // Close the server socket (this won't be reached in this example)
    closesocket(server_sock);
    WSACleanup();

    return 0;
}
