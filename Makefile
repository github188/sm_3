CROSS_COMPILE:= arm-hisiv100nptl-linux-
ARCH:= arm
CC:= $(CROSS_COMPILE)gcc
LD:= $(CROSS_COMPILE)ld

objects = p4storage.o \
		  p4storagefun.o common.o

p4storage:$(objects)
	$(CC) -o srm $(objects) -lpthread -Wall -g

clean:
	rm *~ $(objects) srm -rf
	#rm video/* -rf
	#rm index/* -rf
	#rm /mnt/hgfs/share/storage_module -rf
	#cp ../storage_module /mnt/hgfs/share/ -rf
