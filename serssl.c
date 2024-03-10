#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

SSL_CTX *create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx) {
    SSL_CTX_set_ecdh_auto(ctx, 1);

    // Set the certificate file and private key file
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

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

    // Initialize SSL
    SSL_CTX *ssl_ctx = create_context();
    configure_context(ssl_ctx);

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

        // Print a message when a client is connected
        printf("Client connected!\n");

        // Create new SSL connection
        SSL *ssl = SSL_new(ssl_ctx);
        SSL_set_fd(ssl, client_sock);

        // Perform SSL handshake
        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            closesocket(client_sock);
            continue;
        }

        // Send a welcome message to the client using SSL
        SSL_write(ssl, buffer, strlen(buffer));

        // Receive and display messages from the client using SSL
        while (1) {
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytes_received = SSL_read(ssl, buffer, sizeof(buffer));
            if (bytes_received <= 0) {
                perror("Connection closed");
                break;
            }

            printf("[*] Client Message: %s", buffer);

            // Send a response back to the client using SSL
            SSL_write(ssl, "Message received by the server.\n", 32);
        }

        // Print a message when a client is disconnected
        printf("Client disconnected!\n");

        // Close the SSL connection
        SSL_shutdown(ssl);
        SSL_free(ssl);

        // Close the client socket
        closesocket(client_sock);
    }

    // Close the server socket (this won't be reached in this example)
    closesocket(server_sock);
    WSACleanup();

    return 0;
}
