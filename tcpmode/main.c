#include "server.h"

int main()
{
	IpInfo* iplist = (IpInfo*)malloc(1 * sizeof(IpInfo));
	Loca** lolist = (Loca**)malloc(1 * sizeof(Loca*)); 
	lolist[0] = (Loca* )malloc(1 * sizeof(Loca));
	pthread_t tid;
	pthread_create(&tid, NULL, working, NULL); 
	
return 0;
}
