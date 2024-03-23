#include "server.h"

int main()
{
	iplist = (IpInfo*)malloc(1 * sizeof(IpInfo));
	lolist = (Loca**)malloc(1 * sizeof(Loca*)); 
	lolist[0] = (Loca* )malloc(1 * sizeof(Loca));
	pthread_t tid;
	pthread_create(&tid, NULL, working, NULL); 
	pthread_detach(tid);
	while(1)
	{
		sleep(1);
	}
	return 0;
}
