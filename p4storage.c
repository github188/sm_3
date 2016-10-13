#include "p4storage.h"

void *p4_heart_fun(void *arg)
{
	p4_heart();

	return;
}

void *p4_video_fun(void *arg)
{
	key_t index_mem_key = 2234;
	size_t index_mem_size = SHM_IND_TOTAL_SIZE;
	key_t frame_mem_key = 1234;
	size_t frame_mem_size = SHM_DAT_TOTAL_SIZE;

	p4_video(index_mem_key, index_mem_size, frame_mem_key, frame_mem_size);

	return;
}

void *p4_terminal_fun(void *arg)
{
	p4_terminal();

	return;
}

void *p4_log_fun(void *arg)
{
	p4_log_collect();
	
	return;
}

int main(void)
{
	int ret = 0;

	pthread_t p4_heart_id;
	pthread_t p4_video_id;
	pthread_t p4_terminal_id;
	pthread_t p4_log_id;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	p4_storage_init();

	ret = pthread_create(&p4_heart_id, &attr, p4_heart_fun, NULL);	
	if (ret != 0)
	{
		perror("create p4 heart thread fail:");
		p4_log(STORAGE_RUN_LOG, "create p4 heart thread fail.");
		exit(EXIT_FAILURE);
	}
	
	ret = pthread_create(&p4_video_id, &attr, p4_video_fun, NULL);
	if (ret != 0)
	{
		perror("create p4 video thread fail:");
		p4_log(STORAGE_RUN_LOG, "create p4 video thread fail.\n");
		exit(EXIT_FAILURE);
	}

	ret = pthread_create(&p4_terminal_id, &attr, p4_terminal_fun, NULL);	
	if (ret != 0)
	{
		perror("create p4 terminal thread fail:");
		p4_log(STORAGE_RUN_LOG, "create p4 terminal thread fail.\n");
		exit(EXIT_FAILURE);
	}

	ret = pthread_create(&p4_log_id, &attr, p4_log_fun, NULL);	
	if (ret != 0)
	{
		perror("create p4 log thread fail:");
		p4_log(STORAGE_RUN_LOG, "create p4 log thread fail.\n");
		exit(EXIT_FAILURE);
	}

	for (; ;)
	{
		pause();
	}

	return 0;
}

