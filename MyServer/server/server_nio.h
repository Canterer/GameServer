/**
* Copyright (C) 2008 Happy Fish / YuQing
*
* FastDFS may be copied only under the terms of the GNU General
* Public License V3, which may be found in the FastDFS source kit.
* Please visit the FastDFS Home Page http://www.csource.org/ for more detail.
**/

//tracker_nio.h

#ifndef _TRACKER_NIO_H
#define _TRACKER_NIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
//#include "tracker_types.h"
//#include "storage_func.h"
#include "fast_task_queue.h"
#define FDFS_STORAGE_ID_MAX_SIZE	16



#define FDFS_STORAGE_STAGE_NIO_INIT   0
#define FDFS_STORAGE_STAGE_NIO_RECV   1
#define FDFS_STORAGE_STAGE_NIO_SEND   2
#define FDFS_STORAGE_STAGE_NIO_CLOSE  4  //close socket
#define FDFS_STORAGE_STAGE_DIO_THREAD 8

#define FDFS_STORAGE_FILE_OP_READ     'R'
#define FDFS_STORAGE_FILE_OP_WRITE    'W'
#define FDFS_STORAGE_FILE_OP_APPEND   'A'
#define FDFS_STORAGE_FILE_OP_DELETE   'D'
#define FDFS_STORAGE_FILE_OP_DISCARD  'd'

typedef int (*TaskDealFunc)(struct fast_task_info *pTask);

/* this clean func will be called when connection disconnected */
typedef void (*DisconnectCleanFunc)(struct fast_task_info *pTask);

typedef void (*DeleteFileLogCallback)(struct fast_task_info *pTask, \
		const int err_no);

typedef void (*FileDealDoneCallback)(struct fast_task_info *pTask, \
		const int err_no);

typedef int (*FileBeforeOpenCallback)(struct fast_task_info *pTask);
typedef int (*FileBeforeCloseCallback)(struct fast_task_info *pTask);

#define _FILE_TYPE_APPENDER  1
#define _FILE_TYPE_TRUNK     2   //if trunk file, since V3.0
#define _FILE_TYPE_SLAVE     4
#define _FILE_TYPE_REGULAR   8
#define _FILE_TYPE_LINK     16


typedef struct
{
	int nio_thread_index;  //nio thread index
	bool canceled;
	char stage;  //nio stage, send or recv
	char storage_server_id[FDFS_STORAGE_ID_MAX_SIZE];

	//StorageFileContext file_context;

	int64_t total_length;   //pkg total length for req and request
	int64_t total_offset;   //pkg current offset for req and request

	int64_t request_length;   //request pkg length for access log

	//FDFSStorageServer *pSrcStorage;
	TaskDealFunc deal_func;  //function pointer to deal this task
	void *extra_arg;   //store extra arg, such as (BinLogReader *)
	DisconnectCleanFunc clean_func;  //clean function pointer when finished
} StorageClientInfo;

struct server_nio_thread_data
{
	struct nio_thread_data thread_data;
	void* extend;// extend
};

#ifdef __cplusplus
extern "C" {
#endif

void storage_recv_notify_read(int sock, short event, void *arg);
int storage_send_add_event(struct fast_task_info *pTask);

void task_finish_clean_up(struct fast_task_info *pTask);
void add_to_deleted_list(struct fast_task_info *pTask);

#ifdef __cplusplus
}
#endif

#endif

