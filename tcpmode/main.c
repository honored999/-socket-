#include "server.h"

int main()
{
	iplist = (IpInfo*)malloc(1 * sizeof(IpInfo));
	lolist = (Loca**)malloc(1 * sizeof(Loca*)); 
	lolist[0] = (Loca* )malloc(1 * sizeof(Loca));
	memcpy(false_msg.time, "-1", sizeof("10"));
    	memcpy(false_msg.location, "-1, -1", sizeof("100.36, 66.7"));
    	memcpy(false_msg.name, "-1", sizeof("1"));
	pthread_t tid;
	pthread_create(&tid, NULL, working, NULL); 
	pthread_detach(tid);
	while(1)
	{
		sleep(1);
	}
	return 0;
}
