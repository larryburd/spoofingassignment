#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 15555
#define MAX_BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[MAX_BUFFER_SIZE];

    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the server address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Socket bind failed");
        close(sockfd);
        exit(1);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        // Receive a message from the client
        int recv_len = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
        if (recv_len == -1) {
            perror("Receive failed");
            close(sockfd);
            exit(1);
        }

        // Cast the IP address to a char and the port to an unsigned int 16
        char *client_ip = inet_ntoa(client_addr.sin_addr);
        uint16_t client_port = htons(client_addr.sin_port);

        //Print the received message, the client's IP addr, and port num
        buffer[recv_len] = '\0';
        printf("Received message from client: %s\n", buffer);
        printf("Client IP: %s\n", client_ip);
        printf("Client Port Number: %d\n", client_port);
        // Send a response to the client
        const char *response = "Hello from the server!";
        if (sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, client_len) == -1) {
            perror("Sendto failed");
            close(sockfd);
            exit(1);
        }
    }

    close(sockfd);
    return 0;
}
