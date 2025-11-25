obj-m += test_mod00.o
KDIR=/usr/src/linux-headers-6.8.0-1040-raspi

all:
	$(MAKE) -C $(KDIR) M=$(shell pwd) modules

clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) clean
	rm -f *.o