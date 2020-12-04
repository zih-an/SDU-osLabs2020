hdrs = ipc.h
c_src = control.c ipc.c
c_obj = control.o ipc.o
r_src = barber.c ipc.c
r_obj = barber.o ipc.o
w_src = customer.c ipc.c
w_obj = customer.o ipc.o
opts = -g -c

all: control barber customer
control: $(c_obj)
	gcc $(c_obj) -o control
control.o: $(c_src) $(hdrs)
	gcc $(opts) $(c_src)
barber: $(r_obj)
	gcc $(r_obj) -o barber
barber.o: $(r_src) $(hdrs)
	gcc $(opts) $(r_src)

customer: $(w_obj)
	gcc $(w_obj) -o customer
customer.o:$(w_src) $(hdrs)
	gcc $(opts) $(w_src)
clean:
	rm control barber customer *.o
