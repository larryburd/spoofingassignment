#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define DEST_PORT 12345
#define DEST_IP "127.0.0.1"  // Change to the server's IP
#define SPOOFED_IP "192.168.1.100"  // Change to the desired spoofed IP

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
    struct sockaddr_in dest_addr;
    char *message = "Hello from spoofed client!";
    
    // Message payload length
    int payload_len = strlen(message);

    // Raw socket
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        perror("Socket creation error");
        exit(1);
    }

    // Define destination address
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(DEST_PORT);
    inet_pton(AF_INET, DEST_IP, &dest_addr.sin_addr);

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
    inet_pton(AF_INET, DEST_IP, &(ip_hdr->ip_dst));
    ip_hdr->ip_sum = checksum(ip_hdr, IP_HDRLEN);

    // UDP Header
    struct udphdr *udp_hdr = (struct udphdr *)(packet + IP_HDRLEN);
    udp_hdr->uh_sport = htons(12345);  // Spoofed source port
    udp_hdr->uh_dport = htons(DEST_PORT);
    udp_hdr->uh_ulen = htons(UDP_HDRLEN + payload_len);
    udp_hdr->uh_sum = 0;

    // Message payload
    memcpy(packet + IP_HDRLEN + UDP_HDRLEN, message, payload_len);

    // Send packet
    if (sendto(sockfd, packet, packet_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Sendto error");
        exit(1);
    }

    printf("Packet sent with spoofed IP %s\n", SPOOFED_IP);

    close(sockfd);
    free(packet);
    return 0;
}