/**
* Copyright (C) 2008 Happy Fish / YuQing
*
* FastDFS may be copied only under the terms of the GNU General
* Public License V3, which may be found in the FastDFS source kit.
* Please visit the FastDFS Home Page http://www.csource.org/ for more detail.
**/

//storage_service.c

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
// #include "fdfs_define.h"
// #include "logger.h"
// #include "fdfs_global.h"
#include "_sockopt.h"
// #include "shared_func.h"
// #include "pthread_func.h"
// #include "sched_thread.h"
// #include "tracker_types.h"
// #include "tracker_proto.h"
#include "server_service.h"
// #include "storage_func.h"
// #include "storage_sync.h"
#include "server_global.h"
// #include "base64.h"
// #include "hash.h"
// #include "fdht_client.h"
// #include "fdfs_global.h"
// #include "tracker_client.h"
// #include "storage_client.h"
#include "server_nio.h"
#include "protocol.h"
// #include "storage_dio.h"
// #include "storage_sync.h"
// #include "trunk_mem.h"
// #include "trunk_sync.h"
// #include "trunk_client.h"
// #include "ioevent_loop.h"

//storage access log actions
// #define ACCESS_LOG_ACTION_UPLOAD_FILE    "upload"
// #define ACCESS_LOG_ACTION_DOWNLOAD_FILE  "download"
// #define ACCESS_LOG_ACTION_DELETE_FILE    "delete"
// #define ACCESS_LOG_ACTION_GET_METADATA   "get_metadata"
// #define ACCESS_LOG_ACTION_SET_METADATA   "set_metadata"
// #define ACCESS_LOG_ACTION_MODIFY_FILE    "modify"
// #define ACCESS_LOG_ACTION_APPEND_FILE    "append"
// #define ACCESS_LOG_ACTION_TRUNCATE_FILE  "truncate"
// #define ACCESS_LOG_ACTION_QUERY_FILE     "status"


extern int g_fdfs_network_timeout;
extern time_t g_current_time;

pthread_mutex_t g_storage_thread_lock;
int g_storage_thread_count = 0;


static void *work_thread_entrance(void* arg);
static void *accept_thread_entrance(void* arg);

#define STORAGE_STATUE_DEAL_FILE	 123456   //status for read or write file


int server_service_init()
{
#define ALLOC_CONNECTIONS_ONCE 256

	int result;
	int bytes;
	int init_connections;
	struct server_nio_thread_data *pThreadData;
	struct server_nio_thread_data *pDataEnd;
	pthread_t tid;
	pthread_attr_t thread_attr;

	if ((result=init_pthread_lock(&g_storage_thread_lock)) != 0)
	{
		return result;
	}

	// if ((result=init_pthread_lock(&path_index_thread_lock)) != 0)
	// {
	// 	return result;
	// }

	// if ((result=init_pthread_lock(&stat_count_thread_lock)) != 0)
	// {
	// 	return result;
	// }

	if ((result=init_pthread_attr(&thread_attr, g_thread_stack_size)) != 0)
	{
		logError("file: "__FILE__", line: %d, " \
			"init_pthread_attr fail, program exit!", __LINE__);
		return result;
	}

    init_connections = g_max_connections < ALLOC_CONNECTIONS_ONCE ?
        g_max_connections : ALLOC_CONNECTIONS_ONCE;
	if ((result=free_queue_init_ex(g_max_connections, init_connections,
                    ALLOC_CONNECTIONS_ONCE, g_buff_size,
                    g_buff_size, sizeof(StorageClientInfo))) != 0)
	{
		return result;
	}

	bytes = sizeof(struct server_nio_thread_data) * g_work_threads;
	g_nio_thread_data = (struct server_nio_thread_data *)malloc(bytes);
	if (g_nio_thread_data == NULL)
	{
		logError("file: "__FILE__", line: %d, " \
			"malloc %d bytes fail, errno: %d, error info: %s", \
			__LINE__, bytes, errno, STRERROR(errno));
		return errno != 0 ? errno : ENOMEM;
	}
	memset(g_nio_thread_data, 0, bytes);

	g_storage_thread_count = 0;
	pDataEnd = g_nio_thread_data + g_work_threads;
	for (pThreadData=g_nio_thread_data; pThreadData<pDataEnd; pThreadData++)
	{
		if (ioevent_init(&pThreadData->thread_data.ev_puller,
			g_max_connections + 2, 1000, 0) != 0)
		{
			result  = errno != 0 ? errno : ENOMEM;
			logError("file: "__FILE__", line: %d, " \
				"ioevent_init fail, " \
				"errno: %d, error info: %s", \
				__LINE__, result, STRERROR(result));
			return result;
		}
		result = fast_timer_init(&pThreadData->thread_data.timer,
				2 * g_fdfs_network_timeout, g_current_time);
		if (result != 0)
		{
			logError("file: "__FILE__", line: %d, " \
				"fast_timer_init fail, " \
				"errno: %d, error info: %s", \
				__LINE__, result, STRERROR(result));
			return result;
		}

		if (pipe(pThreadData->thread_data.pipe_fds) != 0)
		{
			result = errno != 0 ? errno : EPERM;
			logError("file: "__FILE__", line: %d, " \
				"call pipe fail, " \
				"errno: %d, error info: %s", \
				__LINE__, result, STRERROR(result));
			break;
		}

#if defined(OS_LINUX)
		if ((result=fd_add_flags(pThreadData->thread_data.pipe_fds[0], \
				O_NONBLOCK | O_NOATIME)) != 0)
		{
			break;
		}
#else
		if ((result=fd_add_flags(pThreadData->thread_data.pipe_fds[0], \
				O_NONBLOCK)) != 0)
		{
			break;
		}
#endif
		if ((result=pthread_create(&tid, &thread_attr, \
			work_thread_entrance, pThreadData)) != 0)
		{
			logError("file: "__FILE__", line: %d, " \
				"create thread failed, startup threads: %d, " \
				"errno: %d, error info: %s", \
				__LINE__, g_storage_thread_count, \
				result, STRERROR(result));
			break;
		}
		else
		{
			if ((result=pthread_mutex_lock(&g_storage_thread_lock)) != 0)
			{
				logError("file: "__FILE__", line: %d, " \
					"call pthread_mutex_lock fail, " \
					"errno: %d, error info: %s", \
					__LINE__, result, STRERROR(result));
			}
			g_storage_thread_count++;
			if ((result=pthread_mutex_unlock(&g_storage_thread_lock)) != 0)
			{
				logError("file: "__FILE__", line: %d, " \
					"call pthread_mutex_lock fail, " \
					"errno: %d, error info: %s", \
					__LINE__, result, STRERROR(result));
			}
		}
	}

	pthread_attr_destroy(&thread_attr);

	//last_stat_change_count = g_stat_change_count;

	//DO NOT support direct IO !!!
	//g_extra_open_file_flags = g_disk_rw_direct ? O_DIRECT : 0;
	
	// if (result != 0)
	// {
	// 	return result;
	// }

	return result;
}

void storage_nio_notify(struct fast_task_info *pTask)
{
	StorageClientInfo *pClientInfo;
	struct server_nio_thread_data *pThreadData;
	long task_addr;

	pClientInfo = (StorageClientInfo *)pTask->arg;
	pThreadData = g_nio_thread_data + pClientInfo->nio_thread_index;

	task_addr = (long)pTask;
	if (write(pThreadData->thread_data.pipe_fds[1], &task_addr, \
		sizeof(task_addr)) != sizeof(task_addr))
	{
		int result;
		result = errno != 0 ? errno : EIO;
		logCrit("file: "__FILE__", line: %d, " \
			"call write failed, " \
			"errno: %d, error info: %s", \
			__LINE__, result, STRERROR(result));
		abort();
	}
}

static void *accept_thread_entrance(void* arg)
{
	int server_sock;
	int incomesock;
	struct sockaddr_in inaddr;
	socklen_t sockaddr_len;
	in_addr_t client_addr;
	char szClientIp[IP_ADDRESS_SIZE];
	long task_addr;
	struct fast_task_info *pTask;
	StorageClientInfo *pClientInfo;
	struct server_nio_thread_data *pThreadData;

	server_sock = (long)arg;
	while (g_continue_flag)
	{
		sockaddr_len = sizeof(inaddr);
		incomesock = accept(server_sock, (struct sockaddr*)&inaddr, \
					&sockaddr_len);
		if (incomesock < 0) //error
		{
			if (!(errno == EINTR || errno == EAGAIN))
			{
				logError("file: "__FILE__", line: %d, " \
					"accept failed, " \
					"errno: %d, error info: %s", \
					__LINE__, errno, STRERROR(errno));
			}

			continue;
		}

		client_addr = getPeerIpaddr(incomesock, \
				szClientIp, IP_ADDRESS_SIZE);
		// if (g_allow_ip_count >= 0)
		// {
		// 	if (bsearch(&client_addr, g_allow_ip_addrs, \
		// 			g_allow_ip_count, sizeof(in_addr_t), \
		// 			cmp_by_ip_addr_t) == NULL)
		// 	{
		// 		logError("file: "__FILE__", line: %d, " \
		// 			"ip addr %s is not allowed to access", \
		// 			__LINE__, szClientIp);

		// 		close(incomesock);
		// 		continue;
		// 	}
		// }

		if (tcpsetnonblockopt(incomesock) != 0)
		{
			close(incomesock);
			continue;
		}

		pTask = free_queue_pop();
		if (pTask == NULL)
		{
			logError("file: "__FILE__", line: %d, " \
				"malloc task buff failed", \
				__LINE__);
			close(incomesock);
			continue;
		}

		pClientInfo = (StorageClientInfo *)pTask->arg;
		pTask->event.fd = incomesock;
		pClientInfo->stage = FDFS_STORAGE_STAGE_NIO_INIT;
		pClientInfo->nio_thread_index = pTask->event.fd % g_work_threads;
		pThreadData = g_nio_thread_data + pClientInfo->nio_thread_index;

		strcpy(pTask->client_ip, szClientIp);

		task_addr = (long)pTask;
		if (write(pThreadData->thread_data.pipe_fds[1], &task_addr, \
			sizeof(task_addr)) != sizeof(task_addr))
		{
			close(incomesock);
			free_queue_push(pTask);
			logError("file: "__FILE__", line: %d, " \
				"call write failed, " \
				"errno: %d, error info: %s", \
				__LINE__, errno, STRERROR(errno));
		}
        else
        {
            int current_connections;
            current_connections = __sync_add_and_fetch(&g_storage_stat.connection.
                    current_count, 1);
            // if (current_connections > g_storage_stat.connection.max_count) {
            //     g_storage_stat.connection.max_count = current_connections;
            // }
            // ++g_stat_change_count;
        }
	}

	return NULL;
}
static void *work_thread_entrance(void* arg)
{
	int result;
	struct server_nio_thread_data *pThreadData;

	pThreadData = (struct server_nio_thread_data *)arg;
	


	ioevent_loop(&pThreadData->thread_data, storage_recv_notify_read,
		task_finish_clean_up, &g_continue_flag);
	ioevent_destroy(&pThreadData->thread_data.ev_puller);



	if ((result=pthread_mutex_lock(&g_storage_thread_lock)) != 0)
	{
		logError("file: "__FILE__", line: %d, " \
			"call pthread_mutex_lock fail, " \
			"errno: %d, error info: %s", \
			__LINE__, result, STRERROR(result));
	}
	g_storage_thread_count--;
	if ((result=pthread_mutex_unlock(&g_storage_thread_lock)) != 0)
	{
		logError("file: "__FILE__", line: %d, " \
			"call pthread_mutex_lock fail, " \
			"errno: %d, error info: %s", \
			__LINE__, result, STRERROR(result));
	}

	logDebug("file: "__FILE__", line: %d, " \
		"nio thread exited, thread count: %d", \
		__LINE__, g_storage_thread_count);

	return NULL;
}

void storage_accept_loop(int server_sock)
{
	if (g_accept_threads > 1)
	{
		pthread_t tid;
		pthread_attr_t thread_attr;
		int result;
		int i;

		if ((result=init_pthread_attr(&thread_attr, g_thread_stack_size)) != 0)
		{
			logWarning("file: "__FILE__", line: %d, " \
				"init_pthread_attr fail!", __LINE__);
		}
		else
		{
			for (i=1; i<g_accept_threads; i++)
			{
			if ((result=pthread_create(&tid, &thread_attr, \
				accept_thread_entrance, \
				(void *)(long)server_sock)) != 0)
			{
				logError("file: "__FILE__", line: %d, " \
				"create thread failed, startup threads: %d, " \
				"errno: %d, error info: %s", \
				__LINE__, i, result, STRERROR(result));
				break;
			}
			}

			pthread_attr_destroy(&thread_attr);
		}
	}
	accept_thread_entrance((void *)(long)server_sock);
}

int storage_terminate_threads()
{
	struct server_nio_thread_data *pThreadData;
	struct server_nio_thread_data *pDataEnd;
	struct fast_task_info *pTask;
	StorageClientInfo *pClientInfo;
	long task_addr;
	int quit_sock;

	if (g_nio_thread_data != NULL)
	{
		pDataEnd = g_nio_thread_data + g_work_threads;
		quit_sock = 0;

		for (pThreadData=g_nio_thread_data; pThreadData<pDataEnd; \
				pThreadData++)
		{
			quit_sock--;
			pTask = free_queue_pop();
			if (pTask == NULL)
			{
			logError("file: "__FILE__", line: %d, " \
				"malloc task buff failed, you should " \
				"increase the parameter: max_connections",
				__LINE__);
				continue;
			}

			pClientInfo = (StorageClientInfo *)pTask->arg;
			pTask->event.fd = quit_sock;
			pClientInfo->nio_thread_index = pThreadData - g_nio_thread_data;

			task_addr = (long)pTask;
			if (write(pThreadData->thread_data.pipe_fds[1], &task_addr, \
					sizeof(task_addr)) != sizeof(task_addr))
			{
				logError("file: "__FILE__", line: %d, " \
					"call write failed, " \
					"errno: %d, error info: %s", \
					__LINE__, errno, STRERROR(errno));
			}
		}
	}

        return 0;
}

void storage_service_destroy()
{
	pthread_mutex_destroy(&g_storage_thread_lock);
	//pthread_mutex_destroy(&path_index_thread_lock);
	//pthread_mutex_destroy(&stat_count_thread_lock);
}



int storage_deal_task(struct fast_task_info *pTask)
{
	logCrit("storage_deal_task");
	TrackerHeader *pHeader;
	StorageClientInfo *pClientInfo;
	int result;

	pClientInfo = (StorageClientInfo *)pTask->arg;
	pHeader = (TrackerHeader *)pTask->data;

	switch(pHeader->cmd)
	{
		case STORAGE_PROTO_CMD_UPLOAD_FILE:
			logCrit("body:%s", ((char*)pHeader + sizeof(TrackerHeader)));
			result = 1;
			break;
	// 	// case STORAGE_PROTO_CMD_DOWNLOAD_FILE:
	// 	// 	ACCESS_LOG_INIT_FIELDS();
	// 	// 	result = storage_server_download_file(pTask);
	// 	// 	STORAGE_ACCESS_LOG(pTask, \
	// 	// 		ACCESS_LOG_ACTION_DOWNLOAD_FILE, \
	// 	// 		result);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_GET_METADATA:
	// 	// 	ACCESS_LOG_INIT_FIELDS();
	// 	// 	result = storage_server_get_metadata(pTask);
	// 	// 	STORAGE_ACCESS_LOG(pTask, \
	// 	// 		ACCESS_LOG_ACTION_GET_METADATA, \
	// 	// 		result);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_UPLOAD_FILE:
	// 	// 	ACCESS_LOG_INIT_FIELDS();
	// 	// 	result = storage_upload_file(pTask, false);
	// 	// 	STORAGE_ACCESS_LOG(pTask, \
	// 	// 		ACCESS_LOG_ACTION_UPLOAD_FILE, \
	// 	// 		result);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_UPLOAD_APPENDER_FILE:
	// 	// 	ACCESS_LOG_INIT_FIELDS();
	// 	// 	result = storage_upload_file(pTask, true);
	// 	// 	STORAGE_ACCESS_LOG(pTask, \
	// 	// 		ACCESS_LOG_ACTION_UPLOAD_FILE, \
	// 	// 		result);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_APPEND_FILE:
	// 	// 	ACCESS_LOG_INIT_FIELDS();
	// 	// 	result = storage_append_file(pTask);
	// 	// 	STORAGE_ACCESS_LOG(pTask, \
	// 	// 		ACCESS_LOG_ACTION_APPEND_FILE, \
	// 	// 		result);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_MODIFY_FILE:
	// 	// 	ACCESS_LOG_INIT_FIELDS();
	// 	// 	result = storage_modify_file(pTask);
	// 	// 	STORAGE_ACCESS_LOG(pTask, \
	// 	// 		ACCESS_LOG_ACTION_MODIFY_FILE, \
	// 	// 		result);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_TRUNCATE_FILE:
	// 	// 	ACCESS_LOG_INIT_FIELDS();
	// 	// 	result = storage_do_truncate_file(pTask);
	// 	// 	STORAGE_ACCESS_LOG(pTask, \
	// 	// 		ACCESS_LOG_ACTION_TRUNCATE_FILE, \
	// 	// 		result);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_UPLOAD_SLAVE_FILE:
	// 	// 	ACCESS_LOG_INIT_FIELDS();
	// 	// 	result = storage_upload_slave_file(pTask);
	// 	// 	STORAGE_ACCESS_LOG(pTask, \
	// 	// 		ACCESS_LOG_ACTION_UPLOAD_FILE, \
	// 	// 		result);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_DELETE_FILE:
	// 	// 	ACCESS_LOG_INIT_FIELDS();
	// 	// 	result = storage_server_delete_file(pTask);
	// 	// 	STORAGE_ACCESS_LOG(pTask, \
	// 	// 		ACCESS_LOG_ACTION_DELETE_FILE, \
	// 	// 		result);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_SET_METADATA:
	// 	// 	ACCESS_LOG_INIT_FIELDS();
	// 	// 	result = storage_server_set_metadata(pTask);
	// 	// 	STORAGE_ACCESS_LOG(pTask, \
	// 	// 		ACCESS_LOG_ACTION_SET_METADATA, \
	// 	// 		result);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_QUERY_FILE_INFO:
	// 	// 	ACCESS_LOG_INIT_FIELDS();
	// 	// 	result = storage_server_query_file_info(pTask);
	// 	// 	STORAGE_ACCESS_LOG(pTask, \
	// 	// 		ACCESS_LOG_ACTION_QUERY_FILE, \
	// 	// 		result);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_CREATE_LINK:
	// 	// 	result = storage_create_link(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_SYNC_CREATE_FILE:
	// 	// 	result = storage_sync_copy_file(pTask, pHeader->cmd);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_SYNC_DELETE_FILE:
	// 	// 	result = storage_sync_delete_file(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_SYNC_UPDATE_FILE:
	// 	// 	result = storage_sync_copy_file(pTask, pHeader->cmd);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_SYNC_APPEND_FILE:
	// 	// 	result = storage_sync_append_file(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_SYNC_MODIFY_FILE:
	// 	// 	result = storage_sync_modify_file(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_SYNC_TRUNCATE_FILE:
	// 	// 	result = storage_sync_truncate_file(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_SYNC_CREATE_LINK:
	// 	// 	result = storage_sync_link_file(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_FETCH_ONE_PATH_BINLOG:
	// 	// 	result = storage_server_fetch_one_path_binlog(pTask);
	// 	// 	break;
	// 	// case FDFS_PROTO_CMD_QUIT:
	// 	// 	add_to_deleted_list(pTask);
	// 	// 	return 0;
	// 	// case FDFS_PROTO_CMD_ACTIVE_TEST:
	// 	// 	result = storage_deal_active_test(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_REPORT_SERVER_ID:
	// 	// 	result = storage_server_report_server_id(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_TRUNK_ALLOC_SPACE:
	// 	// 	result = storage_server_trunk_alloc_space(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_TRUNK_ALLOC_CONFIRM:
	// 	// 	result = storage_server_trunk_alloc_confirm(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_TRUNK_FREE_SPACE:
	// 	// 	result = storage_server_trunk_free_space(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_TRUNK_SYNC_BINLOG:
	// 	// 	result = storage_server_trunk_sync_binlog(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_TRUNK_GET_BINLOG_SIZE:
	// 	// 	result = storage_server_trunk_get_binlog_size(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_TRUNK_DELETE_BINLOG_MARKS:
	// 	// 	result = storage_server_trunk_delete_binlog_marks(pTask);
	// 	// 	break;
	// 	// case STORAGE_PROTO_CMD_TRUNK_TRUNCATE_BINLOG_FILE:
	// 	// 	result = storage_server_trunk_truncate_binlog_file(pTask);
	// 	// 	break;
		default:
			logError("file: "__FILE__", line: %d, "  \
				"client ip: %s, unkown cmd: %d", \
				__LINE__, pTask->client_ip, \
				pHeader->cmd);
			result = EINVAL;
			break;
	}

	if (result != STORAGE_STATUE_DEAL_FILE)
	{
		pClientInfo->total_offset = 0;
        	if (result != 0)
       	{
            	pClientInfo->total_length = sizeof(TrackerHeader);
            	//made of myself need
            	pClientInfo->total_length += strlen( (char*)pHeader + sizeof(TrackerHeader) );
            	logCrit(" response pClientInfo->total_length = %d ",pClientInfo->total_length);
        	}
		pTask->length = pClientInfo->total_length;

		pHeader = (TrackerHeader *)pTask->data;
		pHeader->status = result;
		pHeader->cmd = STORAGE_PROTO_CMD_RESP;
		long2buff(pClientInfo->total_length - sizeof(TrackerHeader), \
				pHeader->pkg_len);
		storage_send_add_event(pTask);
	}

	return result;
	//return 0;
}