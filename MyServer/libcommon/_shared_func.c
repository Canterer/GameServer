/**
* Copyright (C) 2008 Happy Fish / YuQing
*
* FastDFS may be copied only under the terms of the GNU General
* Public License V3, which may be found in the FastDFS source kit.
* Please visit the FastDFS Home Page http://www.csource.org/ for more detail.
**/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <math.h>

#include "_shared_func.h"
#include "_logger.h"
//#include "sockopt.h"

#ifdef OS_LINUX
#include <sys/sysinfo.h>
#else
#ifdef OS_FREEBSD
#include <sys/sysctl.h>
#endif
#endif

char *toLowercase(char *src)
{
	char *p;
	
	p = src;
	while (*p != '\0')
	{
		if (*p >= 'A' && *p <= 'Z')
		{
			*p += 32;
		}
		p++;
	}
	
	return src;
}

char *toUppercase(char *src)
{
	char *p;
	
	p = src;
	while (*p != '\0')
	{
		if (*p >= 'a' && *p <= 'z')
		{
			*p -= 32;
		}
		p++;
	}
	
	return src;	
}

char *formatDatetime(const time_t nTime, \
	const char *szDateFormat, \
	char *buff, const int buff_size)
{
	static char szDateBuff[128];
	struct tm tmTime;
	int size;

	localtime_r(&nTime, &tmTime);
	if (buff == NULL)
	{
		buff = szDateBuff;
		size = sizeof(szDateBuff);
	}
	else
	{
		size = buff_size;
	}

	*buff = '\0';
	strftime(buff, size, szDateFormat, &tmTime);
	
	return buff;
}

int getCharLen(const char *s)
{
	unsigned char *p;
	int count = 0;
	
	p = (unsigned char *)s;
	while (*p != '\0')
	{
		if (*p > 127)
		{
			if (*(++p) != '\0')
			{
				p++;
			}
		}
		else
		{
			p++;
		}
		
		count++;
	}
	
	return count;
}

void set_log_level(char *pLogLevel)
{
	if (pLogLevel != NULL)
	{
		toUppercase(pLogLevel);
		if ( strncmp(pLogLevel, "DEBUG", 5) == 0 || \
		     strcmp(pLogLevel, "LOG_DEBUG") == 0)
		{
			g_log_context.log_level = LOG_DEBUG;
		}
		else if ( strncmp(pLogLevel, "INFO", 4) == 0 || \
		     strcmp(pLogLevel, "LOG_INFO") == 0)
		{
			g_log_context.log_level = LOG_INFO;
		}
		else if ( strncmp(pLogLevel, "NOTICE", 6) == 0 || \
		     strcmp(pLogLevel, "LOG_NOTICE") == 0)
		{
			g_log_context.log_level = LOG_NOTICE;
		}
		else if ( strncmp(pLogLevel, "WARN", 4) == 0 || \
		     strcmp(pLogLevel, "LOG_WARNING") == 0)
		{
			g_log_context.log_level = LOG_WARNING;
		}
		else if ( strncmp(pLogLevel, "ERR", 3) == 0 || \
		     strcmp(pLogLevel, "LOG_ERR") == 0)
		{
			g_log_context.log_level = LOG_ERR;
		}
		else if ( strncmp(pLogLevel, "CRIT", 4) == 0 || \
		     strcmp(pLogLevel, "LOG_CRIT") == 0)
		{
			g_log_context.log_level = LOG_CRIT;
		}
		else if ( strncmp(pLogLevel, "ALERT", 5) == 0 || \
		     strcmp(pLogLevel, "LOG_ALERT") == 0)
		{
			g_log_context.log_level = LOG_ALERT;
		}
		else if ( strncmp(pLogLevel, "EMERG", 5) == 0 || \
		     strcmp(pLogLevel, "LOG_EMERG") == 0)
		{
			g_log_context.log_level = LOG_EMERG;
		}
	}
}

static inline int do_lock_file(int fd, int cmd)
{
    struct flock lock;
    int result;

    memset(&lock, 0, sizeof(lock));
    lock.l_type = cmd;
    lock.l_whence = SEEK_SET;
    do
    {
        if ((result=fcntl(fd, F_SETLKW, &lock)) != 0)
        {
            result = errno != 0 ? errno : ENOMEM;
            fprintf(stderr, "call fcntl fail, "
                   "errno: %d, error info: %s\n",
                   result, STRERROR(result));
        }
    } while (result == EINTR);

    return result;
}

bool fileExists(const char *filename)
{
	return access(filename, 0) == 0;
}

void long2buff(int64_t n, char *buff)
{
	unsigned char *p;
	p = (unsigned char *)buff;
	*p++ = (n >> 56) & 0xFF;
	*p++ = (n >> 48) & 0xFF;
	*p++ = (n >> 40) & 0xFF;
	*p++ = (n >> 32) & 0xFF;
	*p++ = (n >> 24) & 0xFF;
	*p++ = (n >> 16) & 0xFF;
	*p++ = (n >> 8) & 0xFF;
	*p++ = n & 0xFF;
}

int64_t buff2long(const char *buff)
{
	unsigned char *p;
	p = (unsigned char *)buff;
	return  (((int64_t)(*p)) << 56) | \
		(((int64_t)(*(p+1))) << 48) |  \
		(((int64_t)(*(p+2))) << 40) |  \
		(((int64_t)(*(p+3))) << 32) |  \
		(((int64_t)(*(p+4))) << 24) |  \
		(((int64_t)(*(p+5))) << 16) |  \
		(((int64_t)(*(p+6))) << 8) | \
		((int64_t)(*(p+7)));
}

int fcntl_add_flags(int fd, int get_cmd, int set_cmd, int adding_flags)
{
	int flags;

	flags = fcntl(fd, get_cmd, 0);
	if (flags < 0)
	{
		logError("file: "__FILE__", line: %d, " \
			"fcntl fail, errno: %d, error info: %s.", \
			__LINE__, errno, STRERROR(errno));
		return errno != 0 ? errno : EACCES;
	}

	if (fcntl(fd, set_cmd, flags | adding_flags) == -1)
	{
		logError("file: "__FILE__", line: %d, " \
			"fcntl fail, errno: %d, error info: %s.", \
			__LINE__, errno, STRERROR(errno));
		return errno != 0 ? errno : EACCES;
	}

	return 0;
}

int fd_add_flags(int fd, int adding_flags)
{
    return fcntl_add_flags(fd, F_GETFL, F_SETFL, adding_flags);
}

int file_read_lock(int fd)
{
    return do_lock_file(fd, F_RDLCK);
}

int file_write_lock(int fd)
{
    return do_lock_file(fd, F_WRLCK);
}

int file_unlock(int fd)
{
    return do_lock_file(fd, F_UNLCK);
}