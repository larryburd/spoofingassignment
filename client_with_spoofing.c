#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define MAX_BUFFER_SIZE 1024

//#define SPOOFED_IP "192.168.1.100"
#define SPOOFED_IP "192.168.86.23"
#define SPOOFED_PORT 15555

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE];

    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    printf("Enter a message to send to the server: ");
    fgets(buffer, MAX_BUFFER_SIZE, stdin);

    // Send the message to the server
    // if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    //     perror("Sendto failed");
    //     close(sockfd);
    //     exit(1);
    // }

    int datalen = strlen(buffer);

    // Construct IP header
    struct ip ip_header;
    ip_header.ip_hl = sizeof(struct ip) / 4;
    ip_header.ip_v = 4;
    ip_header.ip_tos = 0;
    ip_header.ip_len = htons(sizeof(struct ip) + sizeof(struct udphdr) + datalen); //May need to change datalen to strlen(buffer)
    ip_header.ip_id = 0;
    ip_header.ip_off = 0;
    ip_header.ip_ttl = IPDEFTTL;
    ip_header.ip_p = IPPROTO_UDP;
    ip_header.ip_sum = 0;

    // Source IP
    struct in_addr src_ip;
    src_ip.s_addr = inet_addr(SPOOFED_IP);
    ip_header.ip_src = src_ip;

    // Destination IP
    struct in_addr dst_ip;
    dst_ip.s_addr = inet_addr(SERVER_IP);
    ip_header.ip_dst = dst_ip;

    // UDP Header
    struct udphdr udp_header;
    udp_header.uh_sport = htons(SPOOFED_PORT);
    udp_header.uh_dport = htons(PORT);
    udp_header.uh_ulen = htons(sizeof(struct udphdr) + datalen);
    udp_header.uh_sum = 0;

    // Construct datagram
    int datagram_size = sizeof(struct ip) + sizeof(struct udphdr) + datalen;
    unsigned char datagram[datagram_size];
    memcpy(datagram, &ip_header, sizeof(struct ip));
    memcpy(datagram+sizeof(struct ip), &udp_header, sizeof(struct udphdr));
    memcpy(datagram+sizeof(struct ip)+sizeof(struct udphdr), buffer, datalen);

    // Sendto() destination information
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    
    // Send the message to the server
    if (sendto(sockfd, datagram, datagram_size, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
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
