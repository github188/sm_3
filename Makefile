CROSS_COMPILE:= arm-hisiv100nptl-linux-
ARCH:= arm
CC:= $(CROSS_COMPILE)gcc
LD:= $(CROSS_COMPILE)ld

objects = p4storage.o \
		  p4storagefun.o common.o print.o

p4storage:$(objects)
	$(CC) -o srm $(objects) -lpthread -Wall -g

clean:
	rm *~ $(objects) srm -rf
	#rm video/* -rf
	#rm index/* -rf
	rm /mnt/hgfs/share/sm_3.4.2.3 -rf
	cp ../sm_3.4.2.3 /mnt/hgfs/share/ -rf
