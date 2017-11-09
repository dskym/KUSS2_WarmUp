#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_LEN 65535

void* callFunc(void* port)
{
    struct sockaddr_in sock_addr;
    int sock_fd;
    int fd;
    char filename[6];
    char message[BUF_LEN];
    char content[BUF_LEN];

    struct tm *tm;
    struct timeval tv;

    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Client : Can't open stream socket\n");
        exit(0);
    }

    printf("port : %u\n", *(in_port_t*)port);

    memset(&sock_addr, 0, sizeof(struct sockaddr_in));

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(*(in_port_t*)port);
    sock_addr.sin_addr.s_addr = inet_addr("10.0.2.15");

    if(connect(sock_fd, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("Connection failed\n");
        exit(0);
    }

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    sprintf(filename, "%u.txt", *(in_port_t*)port);

    fd = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0644);

    while(1)
    {
        recv(sock_fd, message, sizeof(message), 0);

        sprintf(content, "%02d:%02d:%02d.%04d %u %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec, strlen(message), message);

        write(fd, content, strlen(content));
    }

    return NULL;
}

int main(int argc, char* argv[])
{
    pthread_t thread_t[5];
    int status;
    in_port_t port[5] = {11111, 22222, 33333, 44444, 55555};

    for(int i=0;i<5;++i)
    {
        if(pthread_create(&thread_t[i], NULL, callFunc, (void*)&port[i]) < 0)
        {
            printf("Thread Create Error\n");
            exit(0);
        }
    }

    for(int i=0;i<5;++i)
    {
        pthread_join(thread_t[i], (void**)status);
    }

    return 0;
}