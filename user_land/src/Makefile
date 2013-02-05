CC=gcc
CFLAGS= -I ../include/ -ggdb -Wall -pedantic
SRCS=rxfe.c dio_control.c pmat.c dio_tx_ops.c
OBJS=rxfe.o dio_control.o pmat.o dio_tx_ops.o
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