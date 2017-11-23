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
#define FORMAT_LEN 20
//서버에서 전송하는 Message의 최대 길이보다 조금 크게 버퍼 크기를 정함.

char* serverip;
//서버 ip를 전역변수로 선언해서 각각의 thread에 인자로 넘겨주지 않고 사용함.


//thread에서 수행하는 함수
void* callFunc(void* port)
{
//각 port마다 socket을 생성해서 서버에 연결한다.
    struct sockaddr_in sock_addr;
    int sock_fd;
    int fd;
    int recvlen;
    char filename[6];
    char message[BUF_LEN];
    char content[BUF_LEN + FORMAT_LEN];

    struct tm *tm;
    struct timeval tv;


//소켓을 생성한다.
    if((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Error : Can't open stream socket\n");
        pthread_exit((void*)-1);
    }

    printf("port : %u\n", *(in_port_t*)port);

    memset(&sock_addr, 0, sizeof(struct sockaddr_in));

//서버와 연결을 위해 필요한 구조체를 초기화한다.
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(*(in_port_t*)port);
    sock_addr.sin_addr.s_addr = inet_addr(serverip);

//서버와 소켓을 연결한다.
    if(connect(sock_fd, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("Error: Server Unreachable\n");
        pthread_exit((void*)-1);
    }

//port별로 file name을 설정한다.
    sprintf(filename, "%u.txt", *(in_port_t*)port);

//file name을 가진 file을 생성하고 open한다.
    fd = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0644);

//서버에서 데이터를 전송받고 파일에 전송받은 데이터를 쓴다.
    while(1)
    {
//서버에서 데이터를 수신한다.
        recvlen = recv(sock_fd, message, sizeof(message)-1, 0);

//현재 시간을 가져온다.
        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);

        if(recvlen == 0){
//Server가 close되면 반복문을 빠져나간다.
            printf("Server closed %d\n", *(in_port_t*)port);
            break;
        }else if(recvlen < 0){
//수신에 Error가 있으면 반복문을 빠져나간다.
            printf("Error: Recv Failed!\n");
            break;
        }

        printf("%d %02d.%04ld, %d\n", *(in_port_t*)port, tm->tm_sec, tv.tv_usec, recvlen);

//수신한 message 뒤에 문자열 끝을 표시할 '\0'를 붙이고, 파일에 쓰기 위해서 formatting을 한다.
        message[recvlen] = '\0';
        sprintf(content, "%02d:%02d:%02d.%04ld %lu %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec, strlen(message), message);


//서버에서 Message를 수신한 시간과 Message의 길이, Message를 파일에 쓴다.
        write(fd, content, strlen(content));
    }


//서버와의 연결이 종료되면 소켓과 파일을 close합니다.
    close(fd);
    close(sock_fd);

//thread를 종료한다
//만약 recv중에 에러가 발생하면 음수(에러코드) 반환하고
//에러 안났으면 0을 반환한다.
    pthread_exit((void*)recvlen);
}

int main(int argc, char* argv[])
{
    pthread_t thread_t[5];
    int status;
    in_port_t port[5] = {11111, 22222, 33333, 44444, 55555};
    serverip = argv[1];

//ip가 인자로 입력되지 않았으면 사용법을 출력한다.
    if(argc != 2){
        printf("Error: Server ip is not given\n");
        printf("Usage: %s <server-ip>\n", argv[0]);
        exit(-1);
    }


//port별로 thread를 생성한다.
    for(int i=0;i<5;++i)
    {
        if(pthread_create(&thread_t[i], NULL, callFunc, (void*)&port[i]) < 0)
        {
            printf("Thread Create Error\n");
            exit(-1);
        }
    }


//thread가 다 끝날때까지 기다린다
//status에는 각 thread에서 return한 값을 저장하고, 출력한 뒤에 프로그램을 끝낸다.
    for(int i=0;i<5;++i)
    {
        pthread_join(thread_t[i], (void**)(&status));
        printf("Thread %d joined! ret: %d\n", i, status);
    }

    return 0;
}
