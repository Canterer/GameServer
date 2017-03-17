.SUFFIXES: .c .o .lo

COMPILE = $(CC) -Wall -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -g -O -DDEBUG_FLAG
ENABLE_STATIC_LIB = 0
ENABLE_SHARED_LIB = 1

INC_PATH = -I$(PWD)/common
INC_PATH += -I$(PWD)/server
INC_PATH += -I$(PWD)/libcommon
 
LIB_PATH = -L$(PWD)/libs -lpthread

TARGET_PATH = /usr/bin
TARGET_LIB = /usr/lib64
TARGET_INC = /usr/include
CONFIG_PATH = /etc/fdfs

FDFS_STATIC_OBJS = $(PWD)/server/server_global.o 
FDFS_STATIC_OBJS += $(PWD)/server/server_nio.o 
FDFS_STATIC_OBJS += $(PWD)/server/server_service.o 
FDFS_STATIC_OBJS += $(PWD)/common/fdfs_global.o 
FDFS_STATIC_OBJS += $(PWD)/common/protocol.o 
FDFS_STATIC_OBJS += $(PWD)/libcommon/_logger.o 
FDFS_STATIC_OBJS += $(PWD)/libcommon/_shared_func.o 
FDFS_STATIC_OBJS += $(PWD)/libcommon/hash.o 
FDFS_STATIC_OBJS += $(PWD)/libcommon/pthread_func.o 
FDFS_STATIC_OBJS += $(PWD)/libcommon/_sockopt.o 
FDFS_STATIC_OBJS += $(PWD)/libcommon/fast_timer.o
FDFS_STATIC_OBJS += $(PWD)/libcommon/ioevent.o
FDFS_STATIC_OBJS += $(PWD)/libcommon/ioevent_loop.o
FDFS_STATIC_OBJS += $(PWD)/libcommon/fast_task_queue.o  

#FDFS_STATIC_OBJS += $(PWD)/main.o

STATIC_OBJS =  $(FDFS_STATIC_OBJS)

#FDFS_SHARED_OBJS = $(PWD)/server/storage_nio.lo 
#FDFS_SHARED_OBJS += $(PWD)//server/storage_service.lo 


FDFS_HEADER_FILES = $(PWD)/common/fdfs_define.h
FDFS_HEADER_FILES += $(PWD)/libcommon/common_define.h 
FDFS_HEADER_FILES += $(PWD)/libcommon/_os_define.h  
#FDFS_HEADER_FILES += $(PWD)/common/tracker_proto.h 
#FDFS_HEADER_FILES += $(PWD)/common/tracker_types.h



ALL_OBJS = $(STATIC_OBJS) $(FDFS_SHARED_OBJS)



STATIC_LIBS = libserver.a
SHARED_LIBS = libserver.so

CLIENT_SHARED_LIBS = libserver.so

ALL_LIBS = $(STATIC_LIBS) $(SHARED_LIBS)


ALL_PRGS = server_main

#all: $(ALL_OBJS) $(ALL_PRGS) $(ALL_LIBS)
#libfdfsclient.so:
#	$(COMPILE) -o $@ $< -shared $(FDFS_SHARED_OBJS) $(LIB_PATH)
#libfdfsclient.a:
#	ar cru $@ $< $(FDFS_STATIC_OBJS)
all: $(ALL_OBJS) $(ALL_PRGS)
.o:
	$(COMPILE) -o $@ $<  $(STATIC_OBJS) $(LIB_PATH) $(INC_PATH)
.c:
	$(COMPILE) -o $@ $<  $(STATIC_OBJS) $(LIB_PATH) $(INC_PATH)
.c.o:
	$(COMPILE) -c -o $@ $<  $(INC_PATH)
#.c.lo:
#	$(COMPILE) -c -fPIC -o $@ $<  $(INC_PATH)
install:
	mkdir -p $(TARGET_PATH)
	mkdir -p $(CONFIG_PATH)
	mkdir -p $(TARGET_LIB)
	mkdir -p /usr/lib
	cp -f $(ALL_PRGS) $(TARGET_PATH)
	if [ 0 -eq 1 ]; then cp -f $(STATIC_LIBS) $(TARGET_LIB); cp -f $(STATIC_LIBS) /usr/lib/;fi
	if [ 1 -eq 1 ]; then cp -f $(CLIENT_SHARED_LIBS) $(TARGET_LIB); cp -f $(CLIENT_SHARED_LIBS) /usr/lib/;fi

	mkdir -p $(TARGET_INC)/fastdfs
	cp -f $(FDFS_HEADER_FILES) $(TARGET_INC)/fastdfs
	if [ ! -f $(CONFIG_PATH)/client.conf.sample ]; then cp -f ../conf/client.conf $(CONFIG_PATH)/client.conf.sample; fi
clean:
	rm -f $(ALL_OBJS) $(ALL_PRGS) $(ALL_LIBS)


