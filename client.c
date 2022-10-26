#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
    if (argc != 3) {
        printf("<port> <URL>\n");
        return 0;
    }

    int sockfd, n;
    int len = sizeof(struct sockaddr);
    char recvline[40960];
    struct sockaddr_in servaddr;
    struct hostent* host;

    /* AF_INET - IPv4 IP , Type of socket, protocol*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));

    if ((host = gethostbyname("cell01-cse.eng.unt.edu")) == NULL) {
        printf("Error retrieving IP\n");
        return;
    }

    servaddr.sin_port = htons(atoi(argv[1]));
    servaddr.sin_family = host->h_addrtype;
    memcpy(&servaddr.sin_addr, host->h_addr, host->h_length);

    /* Connect to the server */
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("Server connection error!\n");
        return;
    }

    // send proxy server the website we want.
    write(sockfd, argv[2], strlen(argv[2])); 
    while (n = read(sockfd, recvline, sizeof(recvline)) > 0)
    {
        printf("%s\n", recvline); // print the received text from server
    }

    close(sockfd);
}