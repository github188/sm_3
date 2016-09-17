#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <error.h>
#include <string.h>
#include "p4storage.h"
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

int main(void)
{
	/*key_t index_mem_key = 2234;
	size_t index_mem_size = SHM_IND_TOTAL_SIZE;
	key_t frame_mem_key = 1234;
	size_t frame_mem_size = SHM_DAT_TOTAL_SIZE;
	p4_video(index_mem_key, index_mem_size, frame_mem_key, frame_mem_size);*/

	p4_terminal();

	return 0;
}

