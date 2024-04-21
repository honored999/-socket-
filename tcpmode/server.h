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
#include <pthread.h>

typedef struct location
{
   char time[20];
   char location[20];
   char name[3];
}Loca;

typedef struct ipinfo
{
   char ip[40];
   int port;
   int id;
   int fd;
   int maxid;
   int nid;
   int real;
   int send;
   Loca sendmsg;
}IpInfo;

typedef struct list
{
  Loca** lolist;
  IpInfo* iplist;
}List;

IpInfo* iplist ;//创建全局变量

Loca** lolist ;//创建全局变量

Loca false_msg;

pthread_rwlock_t rwlock;//创建读写锁

IpInfo* connect_msg(char *ip);//更新客户端列表

void send_msg(IpInfo *ip, Loca data);//向客户端发送数据

Loca recv_msg(IpInfo* ip);//获取指定客户端的最新数据

/*void run_msg();*/

void* working(void* arg);// 运行单线程的子线程

Loca** create_lolist();

IpInfo* create_iplist();

//void create_list(Loca** lolist, IpInfo* iplist);

void setnonblock(int lfd);

//更新maxfd
int updatemaxfd(fd_set fds, int maxfd);

Loca** create_lolist()
{
  Loca** arr = (Loca**)malloc(1 * sizeof(Loca*)); 
  arr[0] = (Loca* )malloc(1 * sizeof(Loca));
  return arr;
}

IpInfo* create_iplist()
{
  IpInfo* arr = (IpInfo*)malloc(1 * sizeof(IpInfo));
  return arr;
}

List create_list(Loca** lolist, IpInfo* iplist, List list)
{
  list.lolist = lolist;
  list.iplist = iplist;
}

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

void* working(void* arg)
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
    int n = 0;
    int num;

    while(1)
    {
	readfds=readfds_bak;
	maxfd = updatemaxfd( readfds, maxfd);	
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	//select函数设置
  	ret = select( maxfd+1, &readfds, NULL, NULL, &timeout);
  	printf("%d\n",ret);
  	
	if( ret==-1)
	{
	   perror("select failed");
 	   exit(1);
 	}
	else if( ret == 0)
	{
	   printf("在%ld秒内无套接字接受信息\n", timeout.tv_sec);
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
		if(cfd == -1&& errno == EAGAIN)
    		{
		  continue;
    		}
        	printf("客户端的IP地址: %s, 端口: %d\n",inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, sizeof(ip)),ntohs(cliaddr.sin_port));
		pthread_rwlock_wrlock(&rwlock);//写锁
		iplist[n].id=n;
		iplist[n].maxid=0;
		iplist[n].fd=cfd;
		iplist[n].nid=0;
		iplist[n].real=1;
		iplist[n].send=0;
		iplist[n].port=ntohs(cliaddr.sin_port);
		memcpy(iplist[n].ip,inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, sizeof(ip)),strlen(inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, sizeof(ip))));
		
		++n;
		if(n!=0)
		{
		  lolist=(Loca**)realloc(lolist, (n+1)*sizeof(Loca*));
		  lolist[n-1]= (Loca*)malloc(1*sizeof(Loca));
		}
		iplist=(IpInfo*)realloc(iplist,(n+1)*sizeof(IpInfo));
		pthread_rwlock_unlock(&rwlock);//关写锁
		setnonblock(cfd);
		if( cfd > maxfd )
		{
		   maxfd = cfd;
		}
		
		FD_SET(cfd,&readfds_bak);
	   }
	   else
	   {
		printf("接受数据\n");
		pthread_rwlock_rdlock(&rwlock);
		for(num=0 ; num <= sizeof(iplist)/sizeof(iplist[0]); ++num)
		{
		  if(i == iplist[num].fd && iplist[num].real == 1)
		  {
			break;
		  }
		}
		pthread_rwlock_unlock(&rwlock);
		// 接收数据
        	char buf[1024];
        	memset(buf, 0, sizeof(buf));
        	int len = recv(i, buf, sizeof(buf), 0);
        	Loca recvdata;
        	memcpy(&recvdata, buf, sizeof(Loca));
		pthread_rwlock_wrlock(&rwlock);
		memcpy(&(lolist[iplist[num].id][iplist[num].nid]),&recvdata,sizeof(Loca));
		
		++(iplist[num].nid);
		if(iplist[num].nid>iplist[num].maxid)
		{
		  lolist[iplist[num].id]=(Loca*)realloc(lolist[iplist[num].id], (iplist[num].nid+1)*sizeof(Loca));
		  ++(iplist[num].maxid);
		}
		pthread_rwlock_unlock(&rwlock);
		
        	if(len == -1)
        	{
            	   perror("recv failed\n");
		   exit(1);             	
		}	
		
		/*send(i, buf, len, 0);*/
		pthread_rwlock_wrlock(&rwlock);
		if(iplist[num].send==1 && iplist[num].real==1 && FD_ISSET( i, &readfds_bak))
		{
			send(iplist[num].fd, (char*)&(iplist[num].sendmsg), sizeof(Loca), 0);
			iplist[num].send = 0;
		}
		pthread_rwlock_unlock(&rwlock);
        	/*if(send(i, buf, len, 0)== -1)
        	{
            	   perror("send failed\n");
            	   close(i);
		   FD_CLR( i, &readfds_bak);
		   printf("客户端%d中断连接", i);
        	}*/	
		pthread_rwlock_wrlock(&rwlock);
		if(len <= 0)
		{
		   close(i);
		   FD_CLR( i, &readfds_bak);
		   iplist[num].real=0;
		   printf("客户端%d中断连接", i);
		}   
		pthread_rwlock_unlock(&rwlock);  	
		/*if(FD_ISSET( i, &readfds_bak))
		{
		  printf("time:%s location:%s name:%s\n", recvdata.time, recvdata.location, recvdata.name);
	  	  send(i, buf, len, 0);
		}*/
		/*if(close(i) == -1) 
	       	{
            	   perror("close failed");
            	   exit(1);       	
		}	
		FD_CLR( i, &readfds_bak);*/
	   }

   
        }
   }
}

IpInfo* connect_msg(char *ip)
{
  pthread_rwlock_rdlock(&rwlock);
  for(int i= 0; i < 90; ++i)
  {
	if(strcmp( ip, iplist[i].ip) ==0 /*&& iplist[i].real == 1 */)
	{
		printf("%d\n",strcmp( ip, iplist[i].ip));
	   pthread_rwlock_unlock(&rwlock);
	   return &(iplist[i]);
	}	
  }
  pthread_rwlock_unlock(&rwlock);
  return NULL;
}

Loca recv_msg(IpInfo* ip)
{
  /*int i = ip->nid;*/
  /*pthread_rwlock_wrlock(&rwlock);*/
	if(ip == NULL)
	{
		return false_msg;
	}
  else if(ip->nid >=1)
  {
  	--(ip->nid);
  	Loca data = lolist[ip->id][ip->nid];
  	/*pthread_rwlock_unlock(&rwlock);*/
  	return data; 
  }
  else
  {	
  	/*pthread_rwlock_unlock(&rwlock);*/
  	return false_msg;
  }	
}

void send_msg(IpInfo *ip, Loca data)
{
  /*pthread_rwlock_wrlock(&rwlock);*/
	if(ip == NULL)
	{
		printf("无对应客户端");
		return;
	}
  else if(ip->real == 1)
  {
	ip->send = 1;
	memcpy(&(ip->sendmsg), &data, sizeof(Loca));
  }
  else
  {
	printf("客户端已断开");
  }
  /*pthread_rwlock_unlock(&rwlock);*/
  return;
}
