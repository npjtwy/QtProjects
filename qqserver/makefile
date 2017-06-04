.SUFFIXES: .cpp .o

CC=g++

SRCS=qqserver.cpp\
 pub.cpp\
 work.cpp\
 mysqlhelper.cpp
OBJS=$(SRCS:.cpp=.o)
LIBDIR+=-L/usr/lib64/mysql
EXE=qqserverd

all:$(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(LIBDIR) -lmysqlclient
	@echo '=========OK========='

.cpp.o:
	$(CC) -std=c++11 -Wall -g -o $@ -c $<


clean:
	-rm -f $(OBJS)
	-rm -f core*
