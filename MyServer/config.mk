CROSS = 

CC =  $(CROSS)gcc
CPLUSPLUS =  $(CROSS)g++
STRIP = $(CROSS)strip
AR = $(CROSS)ar -rv

#CFLAGS= -Wall -march=i686
CFLAGS=  -g -Wall -D_OPENSSL -DLINUX -D_LICENSE -Wno-write-strings


