head = oneway.h
srcs = oneway.cc
objs = oneway.o
opts = -w -g -c
all: oneway
oneway: $(objs)
	g++ $(objs) -o oneway
oneway.o: $(srcs) $(head)
	g++ $(opts) $(srcs)
clean:
	rm oneway *.o
