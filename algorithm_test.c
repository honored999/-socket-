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
static Loca shuju[100];
static Loca sd_msg[100];
static Loca_d shuju_d[100];
static int i = 0;
static IpInfo* msg ;


void  algorithm(int signum);/*声明算法*/

void set_timer()
{
	struct itimerval itv;
	itv.it_interval.tv_sec = 1;
	itv.it_interval.tv_usec = 0;/*1s后计时器运行*/
	itv.it_value.tv_sec = 1;
	itv.it_value.tv_usec = 0;/*s发出一次信号*/
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
	memcpy(ip,"192.168.42.129",sizeof("192.168.42.129"));
	msg=connect_msg(ip);
	while(1)
	{
	if(msg==NULL)
		msg=connect_msg(ip);
	else break;
	}
	sighandler_t sighandler=algorithm;
    	signal(SIGALRM, sighandler);/*定时器每发出一次信号，运行一次algorithm*/
	set_timer();

	while(1){}
	pthread_rwlock_destroy(&rwlock);
	return 0;
}

void algorithm(int signum)
{

	
	struct timeval timein;
	Loca mid;
	int j, k, m = 0;
	long int p=0;
	int x=0;
	while (1) /*接收数据直至为空*/
	{
		gettimeofday(&timein, NULL);
		pthread_rwlock_wrlock(&rwlock);
		shuju[i] = recv_msg(msg);
		pthread_rwlock_unlock(&rwlock);
		
		shuju_d[i].data = shuju[i];
		shuju_d[i].AoI =1000000*timein.tv_sec+ timein.tv_usec - atol(shuju_d[i].data.time);
		shuju_d[i].label=1;
		
		if (strcmp(shuju_d[i].data.time,"-1") != 0)
		{
			i++;
			gettimeofday(&timein, NULL);
			
			pthread_rwlock_wrlock(&rwlock);
			shuju[i] = recv_msg(msg);
			pthread_rwlock_unlock(&rwlock);

			shuju_d[i].data = shuju[i];
			shuju_d[i].AoI = 1000000*timein.tv_sec+timein.tv_usec - atol(shuju_d[i].data.time);
			shuju_d[i].label=1;
			i++;
			
		}
		else
			break;
	}
	
	for(k=0;k<i+1;k++) /*更新AoI*/
		{
			gettimeofday(&timein,NULL);
			shuju_d[i].AoI=1000000*timein.tv_sec+timein.tv_usec-atol(shuju_d[i].data.time);
		}

	for (j = 0; j < i+1; j++) /*排序*/
	{
		for (k = j + 1; k < i+1; k++)
		{
			if (shuju_d[k].AoI < shuju_d[j].AoI)
			{
				mid = shuju_d[j].data;
				shuju_d[j].data = shuju_d[k].data;
				shuju_d[k].data = mid;
				p=shuju_d[j].AoI;
				shuju_d[j].AoI=shuju_d[k].AoI;
				shuju_d[k].AoI=p;
				x=shuju_d[j].label;
				shuju_d[j].label=shuju_d[k].label;
				shuju_d[k].label=x;
			}
		}
	}
	

	for (m = 0; m < i+1; m++)/*发送*/
	{
		if (shuju_d[m].label == 1 /*&& strcmp(shuju_d[i].data.time,"-1") != 0*/)
		{			
			sd_msg[m] = shuju_d[m].data;
			printf("time:%s time_d:%ld location:%s name:%s\n", shuju_d[m].data.time, shuju_d[m].AoI, shuju_d[i].data.location, shuju_d[i].data.name);
			pthread_rwlock_wrlock(&rwlock);
			send_msg(msg, sd_msg[m]);
			pthread_rwlock_unlock(&rwlock);
			
			shuju_d[m].label = 0;
			break;
		}
	}
	

	
}
