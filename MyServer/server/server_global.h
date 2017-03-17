/**
* Copyright (C) 2008 Happy Fish / YuQing
*
* FastDFS may be copied only under the terms of the GNU General
* Public License V3, which may be found in the FastDFS source kit.
* Please visit the FastDFS Home Page http://www.csource.org/ for more detail.
**/

//tracker_nio.h

#ifndef _STORAGE_GLOBAL_H
#define _STORAGE_GLOBAL_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common_define.h"
#include "fdfs_define.h"
// #include "tracker_types.h"
// #include "client_global.h"
// #include "fdht_types.h"
// #include "local_ip_func.h"
// #include "trunk_shared.h"


#define STORAGE_BEAT_DEF_INTERVAL    30
#define STORAGE_REPORT_DEF_INTERVAL  300
#define STORAGE_DEF_SYNC_WAIT_MSEC   100
#define DEFAULT_DISK_READER_THREADS  1
#define DEFAULT_DISK_WRITER_THREADS  1
#define DEFAULT_SYNC_STAT_FILE_INTERVAL  300
#define DEFAULT_DATA_DIR_COUNT_PER_PATH	 256
#define DEFAULT_UPLOAD_PRIORITY           10
#define FDFS_DEFAULT_SYNC_MARK_FILE_FREQ  500
#define STORAGE_DEFAULT_BUFF_SIZE    (64 * 1024)

#define STORAGE_FILE_SIGNATURE_METHOD_HASH  1
#define STORAGE_FILE_SIGNATURE_METHOD_MD5   2


typedef struct
{
	/* following count stat by source server,
           not including synced count
	*/
	int64_t total_upload_count;
	int64_t success_upload_count;
	int64_t total_append_count;
	int64_t success_append_count;
	int64_t total_modify_count;
	int64_t success_modify_count;
	int64_t total_truncate_count;
	int64_t success_truncate_count;
	int64_t total_set_meta_count;
	int64_t success_set_meta_count;
	int64_t total_delete_count;
	int64_t success_delete_count;
	int64_t total_download_count;
	int64_t success_download_count;
	int64_t total_get_meta_count;
	int64_t success_get_meta_count;
	int64_t total_create_link_count;
	int64_t success_create_link_count;
	int64_t total_delete_link_count;
	int64_t success_delete_link_count;
	int64_t total_upload_bytes;
	int64_t success_upload_bytes;
	int64_t total_append_bytes;
	int64_t success_append_bytes;
	int64_t total_modify_bytes;
	int64_t success_modify_bytes;
	int64_t total_download_bytes;
	int64_t success_download_bytes;
	int64_t total_sync_in_bytes;
	int64_t success_sync_in_bytes;
	int64_t total_sync_out_bytes;
	int64_t success_sync_out_bytes;
	int64_t total_file_open_count;
	int64_t success_file_open_count;
	int64_t total_file_read_count;
	int64_t success_file_read_count;
	int64_t total_file_write_count;
	int64_t success_file_write_count;

	/* last update timestamp as source server, 
           current server' timestamp
	*/
	time_t last_source_update;

	/* last update timestamp as dest server, 
           current server' timestamp
	*/
	time_t last_sync_update;

	/* last syned timestamp, 
	   source server's timestamp
	*/
	time_t last_synced_timestamp;

	/* last heart beat time */
	time_t last_heart_beat_time;

    struct {
        int alloc_count;
        volatile int current_count;
        int max_count;
    } connection;
} FDFSStorageStat;

#ifdef __cplusplus
extern "C" {
#endif


extern volatile bool g_continue_flag;

extern int g_server_port;

extern int g_max_connections;
extern int g_accept_threads;
extern int g_work_threads;
extern int g_buff_size;

extern FDFSStorageStat g_storage_stat;

extern LogContext g_access_log_context;


extern char g_bind_addr[IP_ADDRESS_SIZE];
extern bool g_client_bind_addr;
extern bool g_storage_ip_changed_auto_adjust;
extern bool g_thread_kill_done;

extern int g_thread_stack_size;
extern int g_upload_priority;
extern time_t g_up_time;

#if defined(DEBUG_FLAG) && defined(OS_LINUX)
extern char g_exe_name[256];
#endif

extern int g_log_file_keep_days;

extern struct server_nio_thread_data *g_nio_thread_data;  //network io thread data

//int storage_cmp_by_server_id(const void *p1, const void *p2);

#ifdef __cplusplus
}
#endif

#endif