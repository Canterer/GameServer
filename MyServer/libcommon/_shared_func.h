/**
* Copyright (C) 2008 Happy Fish / YuQing
*
* FastDFS may be copied only under the terms of the GNU General
* Public License V3, which may be found in the FastDFS source kit.
* Please visit the FastDFS Home Page http://www.csource.org/ for more detail.
**/

#ifndef SHARED_FUNC_H
#define SHARED_FUNC_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "common_define.h"
//#include "ini_file_reader.h"

#ifdef __cplusplus
extern "C" {
#endif

/** lowercase the string
 *  parameters:
 *  	src: input string, will be changed
 *  return: lowercased string
*/
char *toLowercase(char *src);

/** uppercase the string
 *  parameters:
 *  	src: input string, will be changed
 *  return: uppercased string
*/
char *toUppercase(char *src);


/** date format to string
 *  parameters:
 *  	nTime: unix timestamp
 *  	szDateFormat: date format, more detail man strftime
 *  	buff: store the formated result, can be NULL
 *  	buff_size: buffer size, max bytes can contain
 *  return: formated date string
*/
char *formatDatetime(const time_t nTime, \
	const char *szDateFormat, \
	char *buff, const int buff_size);

/** get character count, only support GB charset
 *  parameters:
 *  	s: the string
 *  return: character count
*/
int getCharLen(const char *s);

/** set global log level
 *  parameters:
 *  	pLogLevel: log level string value
 *  return: none
*/
void set_log_level(char *pLogLevel);

/** check file exist
 *  parameters:
 *  	filename: the filename
 *  return: true if file exists, otherwise false
*/
bool fileExists(const char *filename);

/** long (64 bits) convert to buffer (big-endian)
 *  parameters:
 *  	n: 64 bits int value
 *  	buff: the buffer, at least 8 bytes space, no tail \0
 *  return: none
*/
void long2buff(int64_t n, char *buff);

/** buffer convert to 64 bits int
 *  parameters:
 *  	buff: big-endian 8 bytes buffer
 *  return: 64 bits int value
*/
int64_t buff2long(const char *buff);

/** fcntl add flags such as O_NONBLOCK or FD_CLOEXEC
 *  parameters:
 *  	fd: the fd to set
 *  	get_cmd: the get command
 *  	set_cmd: the set command
 *  	adding_flags: the flags to add
 *  return: error no , 0 success, != 0 fail
*/
int fcntl_add_flags(int fd, int get_cmd, int set_cmd, int adding_flags);

/** set fd flags such as O_NONBLOCK
 *  parameters:
 *  	fd: the fd to set
 *  	adding_flags: the flags to add
 *  return: error no , 0 success, != 0 fail
*/
int fd_add_flags(int fd, int adding_flags);

/** set file read lock
 *  parameters:
 *  	fd: the file descriptor to lock
 *  return: error no, 0 for success, != 0 fail
*/
int file_read_lock(int fd);

/** set file write lock
 *  parameters:
 *  	fd: the file descriptor to lock
 *  return: error no, 0 for success, != 0 fail
*/
int file_write_lock(int fd);

/** file unlock
 *  parameters:
 *  	fd: the file descriptor to unlock
 *  return: error no, 0 for success, != 0 fail
*/
int file_unlock(int fd);

#ifdef __cplusplus
}
#endif

#endif

