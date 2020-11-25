srcs = myppipe.c
objs = myppipe.o
opts = -g -c
all: myppipe
myppipe: $(objs)
	gcc $(objs) -o myppipe
myppipe.o: $(srcs)
	gcc $(opts) $(srcs)
clean:
	rm myppipe *.o