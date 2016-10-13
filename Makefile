CROSS_COMPILE:= arm-hisiv100nptl-linux-
ARCH:= arm
CC:= $(CROSS_COMPILE)gcc
LD:= $(CROSS_COMPILE)ld

objects = p4storage.o \
		  p4storagefun.o common.o

p4storage:$(objects)
	$(CC) -o p4storage $(objects) -lpthread -Wall -g

clean:
	rm *~ $(objects) p4storage -rf
	rm video/* -rf
	rm index/* -rf
	rm /mnt/hgfs/share/storage_module -rf
	cp ../storage_module /mnt/hgfs/share/ -rf
