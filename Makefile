CC = clang-6.0
CFLAGS = -Wall -Wextra -std=c99 -s -O2
TESTFLAGS = -Wall -Wextra -fsanitize=undefined -std=c99 -g -O2

LIB_NAME = harbol

DEPS = -ldl

SRCS = stringobj/stringobj.c
SRCS += vector/vector.c
SRCS += unilist/unilist.c
SRCS += bilist/bilist.c
SRCS += tuple/tuple.c
SRCS += bytebuffer/bytebuffer.c
SRCS += map/map.c
SRCS += allocators/mempool/mempool.c
SRCS += allocators/objpool/objpool.c
SRCS += allocators/cache/cache.c
SRCS += graph/graph.c
SRCS += tree/tree.c
SRCS += linkmap/linkmap.c
SRCS += variant/variant.c
SRCS += cfg/cfg.c
SRCS += plugins/plugins.c
SRCS += veque/veque.c

OBJS = $(SRCS:.c=.o)

harbol_static:
	+$(MAKE) -C stringobj
	+$(MAKE) -C vector
	+$(MAKE) -C unilist
	+$(MAKE) -C bilist
	+$(MAKE) -C tuple
	+$(MAKE) -C bytebuffer
	+$(MAKE) -C map
	+$(MAKE) -C allocators/mempool
	+$(MAKE) -C allocators/objpool
	+$(MAKE) -C allocators/cache
	+$(MAKE) -C graph
	+$(MAKE) -C tree
	+$(MAKE) -C linkmap
	+$(MAKE) -C variant
	+$(MAKE) -C cfg
	+$(MAKE) -C plugins
	+$(MAKE) -C veque
	ar cr lib$(LIB_NAME).a $(OBJS)

harbol_shared:
	+$(MAKE) -C stringobj
	+$(MAKE) -C vector
	+$(MAKE) -C unilist
	+$(MAKE) -C bilist
	+$(MAKE) -C tuple
	+$(MAKE) -C bytebuffer
	+$(MAKE) -C map
	+$(MAKE) -C allocators/mempool
	+$(MAKE) -C allocators/objpool
	+$(MAKE) -C allocators/cache
	+$(MAKE) -C graph
	+$(MAKE) -C tree
	+$(MAKE) -C linkmap
	+$(MAKE) -C variant
	+$(MAKE) -C cfg
	+$(MAKE) -C plugins
	+$(MAKE) -C veque
	$(CC) -shared -o lib$(LIB_NAME).so $(OBJS)

test:
	$(CC) $(TESTFLAGS) $(SRCS) test_suite.c -o $(LIB_NAME)_test $(DEPS)

debug:
	+$(MAKE) -C stringobj debug
	+$(MAKE) -C vector debug
	+$(MAKE) -C unilist debug
	+$(MAKE) -C bilist debug
	+$(MAKE) -C tuple debug
	+$(MAKE) -C bytebuffer debug
	+$(MAKE) -C map debug
	+$(MAKE) -C allocators/mempool debug
	+$(MAKE) -C allocators/objpool debug
	+$(MAKE) -C allocators/cache debug
	+$(MAKE) -C graph debug
	+$(MAKE) -C tree debug
	+$(MAKE) -C linkmap debug
	+$(MAKE) -C variant debug
	+$(MAKE) -C cfg debug
	+$(MAKE) -C plugins debug
	+$(MAKE) -C veque debug
	ar cr lib$(LIB_NAME).a $(OBJS)

debug_shared:
	+$(MAKE) -C stringobj debug
	+$(MAKE) -C vector debug
	+$(MAKE) -C unilist debug
	+$(MAKE) -C bilist debug
	+$(MAKE) -C tuple debug
	+$(MAKE) -C bytebuffer debug
	+$(MAKE) -C map debug
	+$(MAKE) -C allocators/mempool debug
	+$(MAKE) -C allocators/objpool debug
	+$(MAKE) -C allocators/cache debug
	+$(MAKE) -C graph debug
	+$(MAKE) -C tree debug
	+$(MAKE) -C linkmap debug
	+$(MAKE) -C variant debug
	+$(MAKE) -C cfg debug
	+$(MAKE) -C plugins debug
	+$(MAKE) -C veque debug
	$(CC) -shared -o lib$(LIB_NAME).so $(OBJS)

clean:
	+$(MAKE) -C stringobj clean
	+$(MAKE) -C vector clean
	+$(MAKE) -C unilist clean
	+$(MAKE) -C bilist clean
	+$(MAKE) -C tuple clean
	+$(MAKE) -C bytebuffer clean
	+$(MAKE) -C map clean
	+$(MAKE) -C allocators/mempool clean
	+$(MAKE) -C allocators/objpool clean
	+$(MAKE) -C allocators/cache clean
	+$(MAKE) -C graph clean
	+$(MAKE) -C tree clean
	+$(MAKE) -C linkmap clean
	+$(MAKE) -C variant clean
	+$(MAKE) -C cfg clean
	+$(MAKE) -C plugins clean
	+$(MAKE) -C veque clean
	$(RM) *.o
