// server.c
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

//将lfd设置为非阻塞状态
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

//更新maxfd
int updatemaxfd(fd_set fds, int maxfd)
{
    int nmaxfd = 0;
    for(int i = 0; i <= maxfd; ++i)
    {
	if(FD_ISSET( i, &fds) && i > nmaxfd)
	   nmaxfd = i;
    }
    return nmaxfd;
}

int main()
{
    // 1. 创建监听的套接字
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        perror("socket");
        exit(0);
    }

    //socket端口复用
    int optval=1;
    if(setsockopt( lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))) 
    {
	perror("socketopt failed");
	exit(1);
    }
    
    setnonblock(lfd);
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

    //创建select所需的变量 
    fd_set readfds;
    fd_set readfds_bak;
    struct timeval timeout;
    int maxfd = lfd;
    FD_ZERO(&readfds);
    FD_ZERO(&readfds_bak);
    FD_SET(lfd, &readfds_bak);
    // 4. 循环接受客户端连接
    int cfd;
    struct sockaddr_in cliaddr;
    int clilen = sizeof(struct sockaddr_in);
    ret=0;
    char ip[24] = {0};

    while(1)
    {
	readfds=readfds_bak;
	maxfd = updatemaxfd( readfds, maxfd);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	//select函数设置
  	ret = select( maxfd+1, &readfds, NULL, NULL, &timeout);
	if( ret==-1)
	{
	   perror("select failed");
 	   exit(1);
 	}
	else if( ret == 0)
	{
	   printf("在%ld秒内无套接字接受信息", timeout.tv_sec);
	   continue;
	}

	for (int i=0; i <= maxfd; ++i)
	{
	   if(!FD_ISSET( i, &readfds))
	   {
		continue;
	   }
	   if( i == lfd)
	   {
 		int cfd = accept(lfd, (struct sockaddr*)&cliaddr, &clilen);
    		if(cfd == -1)
    		{
		   perror("accept failed\n"); 
	    	   exit(1);
    		}
		/*if(!inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, sizeof(ip)),ntohs(cliaddr.sin_port))    
		{
	       	   perror("inet_ntop failed\n");
	    	   exit(1);
		}*/
        	printf("客户端的IP地址: %s, 端口: %d\n",inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, sizeof(ip)),ntohs(cliaddr.sin_port));
		setnonblock(cfd);
		if( cfd > maxfd )
		{
		   maxfd = cfd;
		}
		FD_SET(cfd,&readfds_bak);
	   }
	   else
	   {
		 // 接收数据
        	char buf[1024];
        	memset(buf, 0, sizeof(buf));
        	int len = recv(i, buf, sizeof(buf), 0);
        	Loca recvdata;
        	memcpy(&recvdata, buf, sizeof(Loca));
        	if(len == -1)
        	{
            	   perror("recv failed\n");
		   exit(1);
              	}
		printf("time:%s location:%s name:%s\n", recvdata.time, recvdata.location, recvdata.name);
        	if(send(i, buf, len, 0)== -1)
        	{
            	   perror("send failed\n");
            	   exit(1);
        	}
        	/*if(close(i) == -1) 
	       	{
            	   perror("close failed");
            	   exit(1);       	
		}	
		FD_CLR( i, &readfds_bak);*/
	   }

   
        }
   }
    return 0;
}


