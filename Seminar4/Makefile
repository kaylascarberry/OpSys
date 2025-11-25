obj-m += test_mod01.o
KDIR=/usr/src/linux-headers-$(shell uname -r)

all:
	$(MAKE) -C $(KDIR) M=$(shell pwd) modules

clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) clean
	rm -f *.o