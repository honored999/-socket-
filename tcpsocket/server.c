// server.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef struct location
{
   char time[20];
   char location[20];
   char name[3];
}Loca;
struct sockinfo
{
   struct sockaddr_in addr;
   int lfd;
};

struct sockinfo infos[512];

void* working(void* arg);

int main()
{
    // 1. 创建监听的套接字
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        perror("socket");
        exit(0);
    }

    // 2. 将socket()返回值和本地的IP端口绑定到一起
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10000);   
    addr.sin_addr.s_addr = INADDR_ANY;  // 这个宏的值为0 == 0.0.0.0
    int ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1)
    {
        perror("bind");
        exit(0);
    }

    // 3. 设置监听
    ret = listen(lfd, 128);
    if(ret == -1)
    {
        perror("listen");
        exit(0);
    }

    //初始化结构体
    int max = sizeof(infos) / sizeof(infos[0]);
    for (int i=0; i < max; ++i)
    {
	bzero(&infos[i] , sizeof(infos[i]));
	infos[i].lfd = -1;
    }

    // 4. 阻塞等待并接受客户端连接
    int clilen = sizeof(struct sockaddr_in);
    while(1)
    {
	struct sockinfo* pinfo;
	for(int i=0; i<max; ++i)
	{
	    if(infos[i].lfd == -1)
	    {
		pinfo = &infos[i];
		break;
	    }
	}
 	int cfd = accept(lfd, (struct sockaddr*)&pinfo->addr, &clilen);
	pinfo->lfd = cfd;
    	if(cfd == -1)
    	{
            perror("accept"); 
	    break;
    	}
	//创建子线程
	pthread_t tid;
	pthread_create(&tid, NULL, working, pinfo); 
	pthread_detach(tid);
    }

    close(lfd);

    return 0;
}

void* working(void* arg)
{
    struct sockinfo* pinfo = (struct sockinfo*)arg;
    // 打印客户端的地址信息
    char ip[24] = {0};
    printf("客户端的IP地址: %s, 端口: %d\n",
           inet_ntop(AF_INET, &pinfo->addr.sin_addr.s_addr, ip, sizeof(ip)),
           ntohs(pinfo->addr.sin_port));

    // 5. 和客户端通信
    while(1)
    {
        // 接收数据
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        int len = recv(pinfo->lfd, buf, sizeof(buf), 0);
        Loca recvdata;
        memcpy(&recvdata, buf, sizeof(Loca));
        if(len > 0)
        {
            printf("time:%s location:%s name:%s\n", recvdata.time, recvdata.location, recvdata.name);
            send(pinfo->lfd, buf, len, 0);
        }
        else if(len  == 0)
        {
            printf("客户端断开了连接...\n");
            break;
        }
        else
        {
            perror("read");
            break;
        }
    }

    close(pinfo->lfd);
    pinfo->lfd = -1;

    return NULL;
}

