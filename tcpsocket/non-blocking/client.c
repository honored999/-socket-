// client.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

typedef struct location
{
   char time[20];
   char location[20];
   char name[3];
}Loca;

void setnonblock(int lfd)
{
    int flags = fcntl( lfd, F_GETFL, 0);
    fcntl( lfd, F_SETFL, flags | O_NONBLOCK);
    if( flags < 0 )
    {  
	perror( "F_GETFL failed");
	exit(1);
    }
    if( fcntl( lfd, F_SETFL, flags | O_NONBLOCK) < 0 )
    {
	perror( "F_SETFL failed");
	exit(1);
    }
}

int main()
{
    // 1. 创建通信的套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
    {
        perror("socket");
        exit(0);
    }
    
    // 2. 连接服务器
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10000);   // 大端端口
    inet_pton(AF_INET, "192.168.42.129", &addr.sin_addr.s_addr);

    int ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1)
    {
        perror("connect");
        exit(0);
    }
    setnonblock(fd);
    Loca data;
    memcpy(data.time, "10", sizeof("10"));
    memcpy(data.location, "100.36, 66.7", sizeof("100.36, 66.7"));
    memcpy(data.name, "1", sizeof("1"));
    char recvbuffer[1024] = {0};
    Loca recvdata;
    // 3. 和服务器端通信
    int number = 0;
    while(1)
    {
        // 发送数据
        send ( fd, (char*)&data,sizeof(Loca), 0 );
        // 接收数据
        int len = recv(fd, recvbuffer, sizeof(recvbuffer), 0);
        if(len > 0)
        {
            memcpy(&recvdata, recvbuffer, sizeof(Loca));
            printf("time:%s location:%s name:%s\n", recvdata.time, recvdata.location, recvdata.name);
        }
        else if(len  == 0)
        {
            printf("服务器断开了连接...\n");
            /*continue;*/
        }
        else
        {
            perror("read");
            /*continue;*/
        }
        sleep(1);   // 每隔1s发送一条数据
    }

    close(fd);

    return 0;
}
