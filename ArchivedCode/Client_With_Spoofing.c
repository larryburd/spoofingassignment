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

// Spoofed Constants
#define SPOOFED_IP "192.168.1.100"

// IP header length
#define IP_HDRLEN 20
#define UDP_HDRLEN 8
#define PSEUDO_HDRLEN 12

// Calculate checksum (IP/UDP checksum algorithm)
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }
    if (len == 1) {
        sum += *(unsigned char *)buf;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}


int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    //char buffer[MAX_BUFFER_SIZE];
    char *buffer;
    char *message = "spoofed message";

    //Create a raw socket
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    printf("Enter a message to send to the server: ");
    fgets(buffer, MAX_BUFFER_SIZE, stdin);

    int payload_len = strlen(buffer);
    //int payload_len = strlen(message);

    // Packet length
    int packet_len = IP_HDRLEN + UDP_HDRLEN + payload_len;
    char *packet = malloc(packet_len);
    if (!packet) {
        perror("Failed to allocate memory for packet");
        exit(1);
    }
    memset(packet, 0, packet_len);

    // IP Header
    struct ip *ip_hdr = (struct ip *)packet;
    ip_hdr->ip_hl = IP_HDRLEN / sizeof(unsigned int);
    ip_hdr->ip_v = 4;
    ip_hdr->ip_tos = 0;
    ip_hdr->ip_len = htons(packet_len);
    ip_hdr->ip_id = htons(0);
    ip_hdr->ip_off = 0;
    ip_hdr->ip_ttl = 64;
    ip_hdr->ip_p = IPPROTO_UDP;
    inet_pton(AF_INET, SPOOFED_IP, &(ip_hdr->ip_src));
    inet_pton(AF_INET, SERVER_IP, &(ip_hdr->ip_dst));
    ip_hdr->ip_sum = checksum(ip_hdr, IP_HDRLEN);

    // UDP Header
    struct udphdr *udp_hdr = (struct udphdr *)(packet + IP_HDRLEN);
    udp_hdr->uh_sport = htons(12345);  // Spoofed source port
    udp_hdr->uh_dport = htons(PORT);
    udp_hdr->uh_ulen = htons(UDP_HDRLEN + payload_len);
    udp_hdr->uh_sum = 0;

    // Message payload
    memcpy(packet + IP_HDRLEN + UDP_HDRLEN, buffer, payload_len);
    //memcpy(packet + IP_HDRLEN + UDP_HDRLEN, message, payload_len);
    // Send the message to the server
    if (sendto(sockfd, packet, packet_len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
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
    free(packet);
    return 0;
}
