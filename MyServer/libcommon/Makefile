.SUFFIXES: .c .o
COMPILE = $(CC) -Wall -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -g -O -DDEBUG_FLAG

#INC_PATH = -I$(PWD)/common
LIB_PATH = -L$(PWD)/ -lpthread

STATIC_OBJS = $(PWD)/_logger.o 
STATIC_OBJS += $(PWD)/_shared_func.o 
STATIC_OBJS += $(PWD)/pthread_func.o 
//STATIC_OBJS += $(PWD)/hash.o 
//STATIC_OBJS += $(PWD)/fast_timer.o 
//STATIC_OBJS += $(PWD)/_sockopt.o

//STATIC_OBJS += $(PWD)/ioevent.o
//STATIC_OBJS += $(PWD)/ioevent_loop.o   

ALL_OBJS = $(STATIC_OBJS)
ALL_PRGS = Test_main

all: $(ALL_OBJS) $(ALL_PRGS) 
.o:
	$(COMPILE) -o $@ $<  $(STATIC_OBJS) $(LIB_PATH) $(INC_PATH)
.c:
	$(COMPILE) -o $@ $<  $(STATIC_OBJS) $(LIB_PATH) $(INC_PATH)
.c.o:
	$(COMPILE) -c -o $@ $<  $(INC_PATH)

clean:
	rm -f $(ALL_OBJS) $(ALL_PRGS)


