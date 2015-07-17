CC=g++
LIB=-lboost_thread -lboost_system
LIBDIR=-L/usr/local/boost/bin -L/opt/local/lib 
INCDIR=-I. -I/usr/local/boost/include -I/opt/local/include
CFLAGS=-Wall
TARGETS=sdlab
SRCS=sdlab.cc sdlab_http.cc sdlab_udpfft.cc fftsg.cc UDPComm.cc sdlab_http_signal.cc sdlab_http_cmd.cc sdlab_http_status.cc
OBJS=$(SRCS:.cc=.o)
DEPS=$(SRCS:.cc=.d)


all: $(TARGETS) $(OBJS)

$(TARGETS): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(INCDIR) $(LIBDIR) $(LIB)

.cc.o:
	$(CC) $(CFLAGS) -c -MMD -MP $< $(INCDIR)


.PHONY: clean
clean:
	$(RM) *~ $(TARGETS) $(OBJS)

-include $(DEPS)
