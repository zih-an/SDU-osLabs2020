hdrs = myipc.h
opts = -g -c
c_src = smoker.c myipc.c
c_obj = smoker.o myipc.o
p_src = producer.c myipc.c
p_obj = producer.o myipc.o
all: producer smoker
smoker: $(c_obj)
	gcc $(c_obj) -o smoker
smoker.o: $(c_src) $(hdrs)
	gcc $(opts) $(c_src)
producer: $(p_obj)
	gcc $(p_obj) -o producer
producer.o: $(p_src) $(hdrs)
	gcc $(opts) $(p_src)
clean:
	rm smoker producer *.o
