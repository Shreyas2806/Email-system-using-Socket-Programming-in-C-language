#include <stdio.h>
#include <winsock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

// Function to set up the SSL/TLS context
SSL_CTX* createSSLContext() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    SSL_CTX* sslContext = SSL_CTX_new(TLS_client_method());
    if (!sslContext) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    return sslContext;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return EXIT_FAILURE;
    }

    SSL_CTX* sslContext = createSSLContext();
    if (!sslContext) {
        WSACleanup();
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
        SSL_CTX_free(sslContext);
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
        SSL_CTX_free(sslContext);
        return EXIT_FAILURE;
    }
    printf("Connected to the server.\n");

    // Set up SSL on the socket
    SSL* ssl = SSL_new(sslContext);
    if (!ssl) {
        ERR_print_errors_fp(stderr);
        closesocket(sock);
        WSACleanup();
        SSL_CTX_free(sslContext);
        return EXIT_FAILURE;
    }

    SSL_set_fd(ssl, sock);
    if (SSL_connect(ssl) != 1) {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        closesocket(sock);
        WSACleanup();
        SSL_CTX_free(sslContext);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < numMessages; ++i) {
        // Send message to the server
        printf("Enter message %d to send: ", i + 1);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';  // Remove newline character
        SSL_write(ssl, buffer, strlen(buffer));

        // Receive and display response from the server
        memset(buffer, 0, sizeof(buffer));
        SSL_read(ssl, buffer, sizeof(buffer));
        printf("Server Response: %s\n", buffer);
    }

    // Close the SSL connection and the socket
    SSL_free(ssl);
    closesocket(sock);
    printf("Disconnected from the server.\n");

    // Clean up OpenSSL
    SSL_CTX_free(sslContext);
    ERR_free_strings();
    EVP_cleanup();

    WSACleanup();

    return 0;
}
