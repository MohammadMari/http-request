#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include<string.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("<port>\n");
        return 0;
    }

    int listen_fd, conn_fd = -1;
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1])); 

    /* AF_INET - IPv4 IP , Type of socket, protocol*/
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        printf("Socket error!\n");
    }

    // https://stackoverflow.com/questions/5592747/bind-error-while-recreating-socket
    int yes = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    /* Binds the above details to the socket */
    if (bind(listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("Bind error!\n");
    }

    /* Start listening to incoming connections */
    if (listen(listen_fd, 10) != 0) {
        printf("listen failed\n");
    }


    while (1)
    {
        // This will be used to receive the website sent in by the user
        // and will receive the response from the website.
        char recvline[40960];
        // clear it
        memset(recvline, 0, sizeof (recvline));

        conn_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL);
        // read incoming line
        read(conn_fd, recvline, sizeof(recvline));

        // seperate host from directory.
        char directory[256];
        int urlIndex = -1;
        int len = strlen(recvline);
        for (int i = 0; i < len; i++) {
            if (recvline[i] == '/' && urlIndex == -1) {
                recvline[i] = '\0';
                urlIndex = 0;
            }
            else if (urlIndex != -1) {
                directory[urlIndex] = recvline[i];
                recvline[i] = '\0';
                urlIndex++;
            }
        }

        // get IP address to the website provided
        struct hostent* host;
        if ((host = gethostbyname(recvline)) == NULL) {
            printf("%s", recvline);
            printf("Error retrieving IP\n");
            close(conn_fd);
            continue;
        }

        struct sockaddr_in req_addr;
        bzero(&req_addr, sizeof(req_addr));
        req_addr.sin_family = AF_INET;
        req_addr.sin_port = htons(80);
        memcpy(&req_addr.sin_addr, host->h_addr, host->h_length);

        // connect to the website
        int outsock = socket(req_addr.sin_family, SOCK_STREAM, 0);
        if (connect(outsock, (struct sockaddr*)&req_addr, sizeof(req_addr)) != 0) {
            printf("Server connection error!\n");
            close(outsock);
            close(conn_fd);
            continue;
        }

        // send the HTTP request
        char get_request[4096];
        snprintf(get_request, 4096, "GET /%s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", directory, recvline);
        write(outsock, get_request, strlen(get_request));

        // receive response from website
        read(outsock, recvline, sizeof(recvline));
        close(outsock);

        // send back the info
        write(conn_fd, recvline, strlen(recvline));        
        close(conn_fd); //close the connection
    }
}