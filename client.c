#include <stdio.h>
#include <winsock2.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return EXIT_FAILURE;
    }

    char ip[15];  // Assuming IPv4, so 15 characters are enough
    int port;
    char buffer[BUFFER_SIZE];
    int numMessages;

    // Get user input for IP address, port, and the number of messages
    printf("Enter the server IP address: ");
    fgets(ip, sizeof(ip), stdin);
    ip[strcspn(ip, "\n")] = '\0';  // Remove newline character

    printf("Enter the server port: ");
    scanf("%d", &port);
    getchar();  // Consume the newline character left in the buffer

    printf("Enter the number of messages to send and receive: ");
    scanf("%d", &numMessages);
    getchar();  // Consume the newline character left in the buffer

    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        perror("[-]Socket error");
        WSACleanup();
        return EXIT_FAILURE;
    }
    printf("[+]TCP client socket created.\n");

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        perror("Connection failed");
        closesocket(sock);
        WSACleanup();
        return EXIT_FAILURE;
    }
    printf("Connected to the server.\n");

    for (int i = 0; i < numMessages; ++i) {
        // Send message to the server
        printf("Enter message %d to send: ", i + 1);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';  // Remove newline character
        send(sock, buffer, strlen(buffer), 0);

        // Receive and display response from the server
        memset(buffer, 0, sizeof(buffer));
        recv(sock, buffer, sizeof(buffer), 0);
        printf("Server Response: %s\n", buffer);
    }

    // Close the socket
    closesocket(sock);
    printf("Disconnected from the server.\n");

    WSACleanup();

    return 0;
}
