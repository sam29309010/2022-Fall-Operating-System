obj-m += kfetch_mod_310551145.o
PWD := $(CURDIR)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	sudo insmod ./kfetch_mod_310551145.ko
	sudo mknod /dev/kfetch c 509 0

unload:
	sudo rmmod kfetch_mod_310551145
	sudo rm /dev/kfetch
