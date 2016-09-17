all:test shm2

test:
	gcc -o test test.c p4storagefun.c
shm2:
	gcc -o shm2 shm2.c p4storagefun.c

#clean:
#	rm	shm1 shm2 $(objects1) $(objects2) *~ a.out -rf

clean:
	rm *~ a.out *.o shm2 test -rf
	rm /mnt/hgfs/share/shm_3 -rf
	cp ../shm_3 /mnt/hgfs/share/ -rf
	rm video/* -rf
	rm index/* -rf
	#ipcrm -M 0x162e
	#ipcrm -M 0x4d2
