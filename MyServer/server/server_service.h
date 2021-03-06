/**
* Copyright (C) 2008 Happy Fish / YuQing
*
* FastDFS may be copied only under the terms of the GNU General
* Public License V3, which may be found in the FastDFS source kit.
* Please visit the FastDFS Home Page http://www.csource.org/ for more detail.
**/

//storage_service.h

#ifndef _SERVER_SERVICE_H_
#define _SERVER_SERVICE_H_

#include "_logger.h"
//include "fdfs_define.h"
#include "fast_task_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int g_storage_thread_count;
extern pthread_mutex_t g_storage_thread_lock;

int server_service_init();
int storage_terminate_threads();
void storage_service_destroy();

//处理收到的命令包  解析其中的协议
int storage_deal_task(struct fast_task_info *pTask);

void storage_accept_loop(int server_sock);
//void storage_nio_notify(struct fast_task_info *pTask);




#ifdef __cplusplus
}
#endif

#endif
