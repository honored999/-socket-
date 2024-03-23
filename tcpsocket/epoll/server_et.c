// server.c
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

struct sockinfo
{
		struct sockaddr_in addr;
		int lfd;
};

typedef struct location
{
		char time[20];
		char location[20];
		char name[3];
}Loca;

struct sockinfo infos[512];


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

		//创建epoll实例
		int epfd = epoll_create(100);
		if(epfd == -1)
		{
				perror("epoll_create failed");
				exit(0);
		}

		struct epoll_event ev;
		ev.events = EPOLLIN | EPOLLET;
		ev.data.fd = lfd;
		epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);

		struct epoll_event evs[1024];
		int size = sizeof(evs)/sizeof(evs[0]);
		//持续监测
		while(1)
		{
				int num = epoll_wait(epfd, evs, size, -1);
				/*printf("%d\n", num);*/
				for(int i = 0; i < num; ++i)
				{
						int fd = evs[i].data.fd;	
						if(fd == lfd)
						{
								int cfd= accept(fd, NULL, NULL);
								//设置非阻塞属性
								int flag = fcntl(cfd, F_GETFL);
								flag |=O_NONBLOCK;
								fcntl(cfd, F_SETFL, flag);
								ev.events = EPOLLIN | EPOLLET;
								ev.data.fd = cfd;
								epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
						}
						else
						{
								// 接收数据
								char buf[1024];
								memset(buf, 0, sizeof(buf));
								while(1)
								{
										int len = recv(fd, buf, sizeof(buf), 0);
										Loca recvdata;
										memcpy(&recvdata, buf, sizeof(Loca));
										if(len > 0)
										{
												printf("time:%s location:%s name:%s\n", recvdata.time, recvdata.location, recvdata.name);
												send(fd, (char*)&recvdata, len, 0);
										}
										else if(len  == 0)
										{
												printf("客户端断开了连接...\n");
												epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
												close(fd);
												break;
										}
										else
										{
												if(errno == EAGAIN)
												{
													printf("接受完毕\n");
													break;
												}
												else
												{
													perror("read");
													exit(1);
												}
												
										}

								}
						}
				}
		}

		close(lfd);

		return 0;
}


