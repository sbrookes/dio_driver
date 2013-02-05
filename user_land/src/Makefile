CC=gcc
CFLAGS= -I ../include/ -ggdb -Wall -pedantic
SRCS=rxfe.c dio_control.c pmat.c
OBJS=rxfe.o dio_control.o pmat.o
EXEC=xdio_control

user: $(EXEC)

$(EXEC) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

$(OBJS) : $(SRCS)
	$(CC) $(CFLAGS) -c $(SRCS)

celan: clean # accomadate a common typo...
clean: 
	rm -f *~
	rm -f *#
	rm -f $(EXEC) $(OBJS)