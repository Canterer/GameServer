/**
* Copyright (C) 2008 Happy Fish / YuQing
*
* FastDFS may be copied only under the terms of the GNU General
* Public License V3, which may be found in the FastDFS source kit.
* Please visit the FastDFS Home Page http://www.csource.org/ for more detail.
**/

#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include "_logger.h"
#include "_sockopt.h"
#include "_shared_func.h"
#include "server_global.h"

volatile bool g_continue_flag = true;

int g_server_port = FDFS_STORAGE_SERVER_DEF_PORT;

int g_max_connections = DEFAULT_MAX_CONNECTONS;
int g_accept_threads = 1;
int g_work_threads = DEFAULT_WORK_THREADS;
int g_buff_size = STORAGE_DEFAULT_BUFF_SIZE;

FDFSStorageStat g_storage_stat;

LogContext g_access_log_context = {LOG_INFO, STDERR_FILENO, NULL};


char g_bind_addr[IP_ADDRESS_SIZE] = {0};
bool g_client_bind_addr = true;
bool g_storage_ip_changed_auto_adjust = false;
bool g_thread_kill_done = false;

int g_thread_stack_size = 512 * 1024;
int g_upload_priority = DEFAULT_UPLOAD_PRIORITY;
time_t g_up_time = 0;


#if defined(DEBUG_FLAG) && defined(OS_LINUX)
char g_exe_name[256] = {0};
#endif

int g_log_file_keep_days = 0;
struct server_nio_thread_data *g_nio_thread_data = NULL;

// int storage_cmp_by_server_id(const void *p1, const void *p2)
// {
// 	return strcmp((*((FDFSStorageServer **)p1))->server.id,
// 		(*((FDFSStorageServer **)p2))->server.id);
// }

