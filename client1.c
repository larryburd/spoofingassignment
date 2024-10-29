#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define MAX_BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE];

    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    printf("Enter a message to send to the server: ");
    fgets(buffer, MAX_BUFFER_SIZE, stdin);

    // Send the message to the server
    if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Sendto failed");
        close(sockfd);
        exit(1);
    }

    // Receive the response from the server
    int recv_len = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, NULL, NULL);
    if (recv_len == -1) {
        perror("Receive failed");
        close(sockfd);
        exit(1);
    }

    buffer[recv_len] = '\0';
    printf("Received response from the server: %s\n", buffer);

    close(sockfd);
    return 0;
}
