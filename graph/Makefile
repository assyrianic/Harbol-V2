CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -s -O2
TESTFLAGS = -Wall -Wextra -pedantic -std=c99 -g -O2

SRCS = graph.c
OBJS = $(SRCS:.c=.o)

harbol_graph:
	$(CC) $(CFLAGS) -c $(SRCS)

debug:
	$(CC) $(TESTFLAGS) -c $(SRCS)

clean:
	$(RM) *.o
