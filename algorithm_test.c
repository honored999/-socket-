#include<stdio.h>
#include <sys/time.h>/*包含：获取当前时间gettimeofday、定时器setitimer*/
#include<signal.h>
#include<stdlib.h>
#include"server.h"


typedef void (*sighandler_t)(int);

typedef struct
{
	Loca data;
	long int AoI;
	int label;/*标记是否已被发送*/
}Loca_d;

static struct itimerval oldtv;/*无意义*/

sighandler_t algorithm(Loca* shuju, Loca* sd_msg, Loca_d* shuju_d,int i);/*声明算法*/

void set_timer()
{
	struct itimerval itv;
	itv.it_interval.tv_sec = 0;
	itv.it_interval.tv_usec = 100000;/*100000us后计时器运行*/
	itv.it_value.tv_sec = 0;
	itv.it_value.tv_usec = 100000;/*计时器每隔100000us发出一次信号*/
	setitimer(ITIMER_REAL, &itv, &oldtv);
}


int main()
{
	Loca shuju[10000];
	Loca sd_msg[10000];
	Loca_d shuju_d[10000];
	int i = 0;
	IpInfo* msg = connect_msg(ip,port);/*创立连接*/
    signal(SIGALRM, algorithm(shuju,sd_msg,shuju_d,i));/*定时器每发出一次信号，运行一次algorithm*/
	set_timer();
	return 0;
}

sighandler_t algorithm(Loca* shuju, Loca* sd_msg, Loca_d* shuju_d,int i)
{
	struct timeval timein;
	Loca mid,test;
	int j, k, m = 0;
	while (1) /*接收数据直至为空*/
	{
		gettimeofday(&timein, NULL);
		shuju[i] = recv_msg(msg);
		shuju_d[i].data = shuju[j];
		shuju_d[i].AoI = timein.tv_usec - atol(shuju[i].time);
		shuju_d[i].label=1;
		if (strcmp(shuju[i].name,-1) != 0)
		{
			i++;
			gettimeofday(&timein, NULL);
			shuju[i] = recv_msg(msg);
			shuju_d[i].data = shuju[j];
			shuju_d[i].AoI = timein.tv_usec - atol(ashuju[i].time);
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
		if (shuju_d[m].label = 1)
		{
			sd_msg[m] = shuju_d[m].data;
			send_msg(ip, sd_msg[m]);
			shuju_d[m].label = 0;
			break;
		}
	}
}
