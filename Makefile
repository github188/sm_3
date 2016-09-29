all:test

test:
	#arm-hisiv100nptl-linux-gcc -o p4storage p4storage.c p4storagefun.c common.c -pthread
	gcc -o p4storage p4storage.c p4storagefun.c common.c -pthread

clean:
	rm *~ *.o -rf
	rm video/* -rf
	rm index/* -rf
	rm /mnt/hgfs/share/storage_module -rf
	cp ../storage_module /mnt/hgfs/share/ -rf
