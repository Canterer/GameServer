#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

//多线程  线程池 线程锁 （pipe考虑下）   
int main()
{
	int result;
	int wait_count;
	int sock;
	pthread_t schedule_tid;
	struct sigaction act;

	//默认3个tracker调度进程
	ScheduleEntry scheduleEntries[SCHEDULE_ENTRIES_COUNT];
	ScheduleArray scheduleArray;

	//记录启动时间
	g_current_time = time(NULL);
	g_up_time = g_current_time;
	srand(g_up_time);


	return result;
}
