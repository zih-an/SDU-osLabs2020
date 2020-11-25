src = mytpipe.c
obj = mytpipe.o
opt = -g -c
all: mytpipe
mytpipe: $(obj)
	gcc $(obj) -l pthread -o mytpipe
mytpipe.o: $(src)
	gcc $(opt) $(src)
clean:
	rm mytpipe *.o