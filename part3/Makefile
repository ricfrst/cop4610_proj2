obj-m += elevator.o
elevator-objs := src/elevator_module.o src/pets.o src/scheduler.o \
                 src/elevator_thread.o src/elevator_proc.o

ccflags-y := -g -Wno-error

KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
