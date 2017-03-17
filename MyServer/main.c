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

#include "_logger.h"
#include "fast_timer.h"
#include "_sockopt.h"
#include "server_service.h"

#define STORAGE_ERROR_LOG_FILENAME      "storaged"
#define DEFAULT_CONNECT_TIMEOUT     		6000
#define DEFAULT_NETWORK_TIMEOUT     		6000
#define DEFAULT_NETWORK_TIMEOUT     		50000
#define IP_ADDRESS_SIZE     				16

int g_fdfs_connect_timeout = DEFAULT_CONNECT_TIMEOUT;
int g_fdfs_network_timeout = DEFAULT_NETWORK_TIMEOUT;
int g_server_port = FDFS_STORAGE_SERVER_DEF_PORT;
char g_bind_addr[IP_ADDRESS_SIZE] = "192.168.169.1";

volatile time_t g_current_time = 0;

//多线程  线程池 线程锁 （pipe考虑下）   
int main(int argc, char *argv[])
{
	int result;
	int wait_count;
	int sock;
	pthread_t schedule_tid;
	//struct sigaction act;

/*
#standard log level as syslog, case insensitive, value list:
### emerg for emergency
### alert
### crit for critical
### error
### warn for warning
### notice
### info
### debug
log_level=info
*/

	//设置日志
	//load_log_level(&iniContext);
	//set_log_level(iniGetStrValue(NULL, "log_level", pIniContext));
	log_init2();
	set_log_level( "LOG_DEBUG" );
	if ((result=log_set_prefix("/home/Canterer/TTT", \
			STORAGE_ERROR_LOG_FILENAME)) != 0)
	{
		logError("logger set prefix failed!");
		return result;
	}
	logCrit("server start!");

	//默认3个tracker调度进程
	//ScheduleEntry scheduleEntries[SCHEDULE_ENTRIES_COUNT];
	//ScheduleArray scheduleArray;

	//记录启动时间
	g_current_time = time(NULL);
	//g_up_time = g_current_time;
	//srand(g_up_time);

	sock = socketServer(g_bind_addr, g_server_port, &result);
	if (sock < 0)
	{
		logCrit("exit abnormally!\n");
		log_destroy();
		return result;
	}

	if ((result=tcpsetserveropt(sock, g_fdfs_network_timeout)) != 0)
	{
		logCrit("exit abnormally!\n");
		log_destroy();
		return result;
	}

	if ((result=server_service_init()) != 0)
	{
		logCrit("file: "__FILE__", line: %d, " \
			"storage_service_init fail, program exit!", __LINE__);
		g_continue_flag = false;
		return result;
	}
	//??
	if ((result=set_rand_seed()) != 0)
	{
		logCrit("file: "__FILE__", line: %d, " \
			"set_rand_seed fail, program exit!", __LINE__);
		g_continue_flag = false;
		return result;
	}

	storage_accept_loop(sock);

	storage_terminate_threads();

	wait_count = 0;
	while (g_storage_thread_count != 0 || \
		g_dio_thread_count != 0 || \
		g_tracker_reporter_count > 0 || \
		g_schedule_flag)
	{
/*
#if defined(DEBUG_FLAG) && defined(OS_LINUX)
		if (bSegmentFault)
		{
			sleep(5);
			break;
		}
#endif
*/
		usleep(10000);
		if (++wait_count > 6000)
		{
			logWarning("waiting timeout, exit!");
			break;
		}
	}

	storage_service_destroy();

	logInfo("exit normally.\n");
	log_destroy();

	return result;
}
