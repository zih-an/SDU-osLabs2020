head = pctl.h
srcs = mypctl.c
objs = mypctl.o
opts = -g -c
all: mypctl
mypctl: $(objs)
	gcc $(objs) -o mypctl
mypctl.o: $(srcs) $(head)
	gcc $(opts) $(srcs)
clean:
	rm mypctl *.o