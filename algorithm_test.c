#include<signal.h>
#include "server.h"

typedef void (*sighandler_t)(int);

typedef struct
{
	Loca data;
	long int AoI;
	int label;/*标记是否已被发送*/
}Loca_d;

static struct itimerval oldtv;/*无意义*/

sighandler_t  algorithm(Loca* shuju, Loca* sd_msg, Loca_d* shuju_d,int i,IpInfo* msg);/*声明算法*/

void set_timer()
{
	struct itimerval itv;
	itv.it_interval.tv_sec = 10;
	itv.it_interval.tv_usec = 1000000;/*100000us后计时器运行*/
	itv.it_value.tv_sec = 0;
	itv.it_value.tv_usec = 1000000;/*计时器每隔100000us发出一次信号*/
	setitimer(ITIMER_REAL, &itv, &oldtv);
}

int main()
{
	iplist = (IpInfo*)malloc(1 * sizeof(IpInfo));
	lolist = (Loca**)malloc(1 * sizeof(Loca*)); 
	lolist[0] = (Loca* )malloc(1 * sizeof(Loca));
	memcpy(false_msg.time, "-1", sizeof("10"));
    	memcpy(false_msg.location, "-1, -1", sizeof("100.36, 66.7"));
     	memcpy(false_msg.name, "-1", sizeof("1"));
	pthread_rwlock_init(&rwlock, NULL);
	pthread_t tid;
	pthread_create(&tid, NULL, working, NULL); 
	pthread_detach(tid);
	
	char ip[40];
	int port = 39728;
	memcpy(ip,"192.168.42.129",sizeof("192.168.42.129"));
	
	Loca shuju[100];
	Loca sd_msg[100];
	Loca_d shuju_d[100];
	int i = 0;
	IpInfo* msg ;/*创立连接*/
    	signal(SIGALRM, algorithm(shuju,sd_msg,shuju_d,i,msg));/*定时器每发出一次信号，运行一次algorithm*/
	set_timer();

		
	
	while(1)
	{
	if(msg == NULL)
		msg = connect_msg(ip);
		
	algorithm(shuju,sd_msg,shuju_d,i,msg);
	sleep(1);
	
	}

	pthread_rwlock_destroy(&rwlock);
	return 0;
}

sighandler_t algorithm(Loca* shuju, Loca* sd_msg, Loca_d* shuju_d,int i,IpInfo* msg)
{
 	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset,SIGALRM);
	sigprocmask(SIG_BLOCK,&sigset,NULL);/*信号屏蔽*/
	
	struct timeval timein;
	Loca mid,test;
	int j, k, m = 0;
	while (1) /*接收数据直至为空*/
	{
		gettimeofday(&timein, NULL);
		
		pthread_rwlock_wrlock(&rwlock);
		shuju[i] = recv_msg(msg);
		pthread_rwlock_unlock(&rwlock);
		
		shuju_d[i].data = shuju[i];
		shuju_d[i].AoI = timein.tv_usec - atol(shuju[i].time);
		shuju_d[i].label=1;
		printf("time:%s location:%s name:%s\n", shuju[i].time, shuju[i].location, shuju[i].name);
		if (strcmp(shuju[i].name,"-1") != 0)
		{
			i++;
			gettimeofday(&timein, NULL);
			
			pthread_rwlock_wrlock(&rwlock);
			shuju[i] = recv_msg(msg);
			pthread_rwlock_unlock(&rwlock);

			shuju_d[i].data = shuju[i];
			shuju_d[i].AoI = timein.tv_usec - atol(shuju[i].time);
			shuju_d[i].label=1;
			
		}
		else
			break;
	}

	for (j = 0; j < i; j++) /*排序*/
	{
		for (k = j + 1; k < i; k++)
		{
			if (shuju_d[k].AoI < shuju_d[j].AoI)
			{
				mid = shuju_d[j].data;
				shuju_d[j].data = shuju_d[k].data;
				shuju_d[k].data = mid;
			}
		}
	}

	for (m = 0; m < i; m++)/*发送*/
	{
		if (shuju_d[m].label == 1)
		{			
			sd_msg[m] = shuju_d[m].data;
			
			pthread_rwlock_wrlock(&rwlock);
			send_msg(msg, sd_msg[m]);
			pthread_rwlock_unlock(&rwlock);
			
			shuju_d[m].label = 0;
			break;
		}
	}
	
	
	sigprocmask(SIG_UNBLOCK,&sigset,NULL);/*信号解除屏蔽*/
}
