#include "server.h"

int main()
{
	pthread_t tid;
	pthread_create(&tid, NULL, working, &list); 
	pthread_detach(tid);

return 0;
}
