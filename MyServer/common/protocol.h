#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <netdb.h>

#define STORAGE_PROTO_CMD_REPORT_SERVER_ID	9  
#define STORAGE_PROTO_CMD_UPLOAD_FILE		11
#define STORAGE_PROTO_CMD_DELETE_FILE		12
#define STORAGE_PROTO_CMD_SET_METADATA		13
#define STORAGE_PROTO_CMD_DOWNLOAD_FILE		14
#define STORAGE_PROTO_CMD_GET_METADATA		15
#define STORAGE_PROTO_CMD_SYNC_CREATE_FILE	16
#define STORAGE_PROTO_CMD_SYNC_DELETE_FILE	17
#define STORAGE_PROTO_CMD_SYNC_UPDATE_FILE	18
#define STORAGE_PROTO_CMD_SYNC_CREATE_LINK	19
#define STORAGE_PROTO_CMD_CREATE_LINK		20
#define STORAGE_PROTO_CMD_UPLOAD_SLAVE_FILE	21
#define STORAGE_PROTO_CMD_QUERY_FILE_INFO	22

#define STORAGE_PROTO_CMD_RESP					100

#define FDFS_PROTO_PKG_LEN_SIZE		8
#define FDFS_PROTO_CMD_SIZE		1
#define FDFS_PROTO_IP_PORT_SIZE		(IP_ADDRESS_SIZE + 6)

typedef struct
{
	char pkg_len[FDFS_PROTO_PKG_LEN_SIZE];  //body length, not including header
	char cmd;    //command code
	char status; //status code for response
} TrackerHeader;

typedef struct
{
	int sock;
	int port;
	char ip_addr[INET6_ADDRSTRLEN];
    int socket_domain;  //socket domain, AF_INET, AF_INET6 or PF_UNSPEC for auto dedect
} ConnectionInfo;

#ifdef __cplusplus
extern "C" {
#endif


int fdfs_recv_header(ConnectionInfo *pTrackerServer, int64_t *in_bytes);

int fdfs_recv_response(ConnectionInfo *pTrackerServer, \
		char **buff, const int buff_size, \
		int64_t *in_bytes);
int fdfs_quit(ConnectionInfo *pTrackerServer);


#ifdef __cplusplus
}
#endif

#endif
