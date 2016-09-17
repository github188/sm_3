#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include "p4storage.h"

void* get_shm(int key, int size)
{
	int shmid = -1;
	key_t tmpkey = (key_t)key;
	size_t tmpsize = (size_t)size;
	void *shared_memory_start = (void *)0;

	shmid = shmget(tmpkey, tmpsize, 0666 | IPC_CREAT);
	if (shmid == -1) 
	{
    	perror("shmget failed\n");
    	exit(EXIT_FAILURE);
	}	

	shared_memory_start = shmat(shmid, (void *)0, 0);
	if (shared_memory_start == (void *)-1) 
	{
    	perror("shmat failed\n");
    	exit(EXIT_FAILURE);
	}

	return shared_memory_start;
}

FILE *open_shm_index(const char *path)
{
	if (path == NULL)
	{
		fprintf(stderr, "shm index file path error\n");
		exit(EXIT_FAILURE);		
	}

	FILE *indexfp = NULL;
	indexfp = fopen(path, "r");
	if (indexfp == NULL)
	{
		perror("fopen shm index file fail:");
		exit(EXIT_FAILURE);
	}	
	
	return indexfp;
}

void close_shm_index(FILE *indexfp)
{
	if (indexfp == NULL)
	{
		fprintf(stderr, "shm index file file descriptor error\n");
		exit(EXIT_FAILURE);				
	}
	fclose(indexfp);
}

int open_tmp(const char* tmpstring)
{
	if (tmpstring == NULL)
	{
		fprintf(stderr, "tmp file string error\n");
		exit(EXIT_FAILURE);				
	}

	int tmp = -1;
	tmp = open(tmpstring, O_RDWR|O_CREAT|O_TRUNC, 0666);
	if (tmp == -1)
	{
		perror(tmpstring);
		exit(EXIT_FAILURE);	
	}

	return tmp;
}

int get_one_shm_index(FILE *indexfp, P4VEM_ShMIndex_t *oldshmindex, P4VEM_ShMIndex_t *newshmindex)
{
	if (indexfp == NULL || oldshmindex == NULL || newshmindex == NULL)
	{
		fprintf(stderr, "get_one_shm_index address error\n");
		exit(EXIT_FAILURE);
	}	

	unsigned char cnt = 0;
	cnt = fread(newshmindex, sizeof(P4VEM_ShMIndex_t), 1, indexfp);	
	if (cnt == 0)
	{
		perror("fread an error  occurs, or the end-of-file is reached.\n");
		return -1;
	}

	if (newshmindex->offset == oldshmindex->offset)
	{
		return -1;
	}
	
	memcpy(oldshmindex, newshmindex, sizeof(P4VEM_ShMIndex_t));

	return 0;
}

void get_one_frame(void *shared_memory_start, P4VEM_ShMIndex_t *cshmindex, FRAME_PACKET *frame)
{
	if (shared_memory_start == NULL || cshmindex == NULL || frame == NULL)
	{
		fprintf(stderr, "get_one_frame address error\n");
		exit(EXIT_FAILURE);
	}	

	memcpy(frame, shared_memory_start+cshmindex->offset, cshmindex->lenth);	

	return;
}

/* Storage_one_frame function only serve one channel for two specified file description, 
 video_tmp_fd and index_tmp_fd*/
int storage_one_frame(void *shared_memory_start, P4VEM_ShMIndex_t *cshmindex, FRAME_PACKET *frame)
{
	if (shared_memory_start == NULL || cshmindex == NULL || frame == NULL)
	{
		fprintf(stderr, "storage_one_frame error\n");
		exit(EXIT_FAILURE);				
	}

	unsigned int shm_read_offset = 0;
    char video_channel_path[PATH_LEN] = {0};
	char index_channel_path[PATH_LEN] = {0};
	char video_day_path[PATH_LEN] = {0};
	char index_day_path[PATH_LEN] = {0};
	char video_tmp[PATH_LEN] = {0};
	char index_tmp[PATH_LEN] = {0};

	sprintf(video_channel_path, "./video/%02d", cshmindex->channel);
	sprintf(index_channel_path, "./index/%02d", cshmindex->channel);
	sprintf(video_day_path, "%s/%02d%02d%02d", video_channel_path,
	                                 cshmindex->time.year, cshmindex->time.month, cshmindex->time.day);
	sprintf(index_day_path, "%s/%02d%02d%02d", index_channel_path,
	                                 cshmindex->time.year, cshmindex->time.month, cshmindex->time.day);
	sprintf(video_tmp, "%s/tmp.h264", video_day_path,
	                                 cshmindex->time.year, cshmindex->time.month, cshmindex->time.day);
	sprintf(index_tmp, "%s/tmp.index", index_day_path,
	                                 cshmindex->time.year, cshmindex->time.month, cshmindex->time.day);
	
	if ((access(video_channel_path, F_OK)) != -1)  
    {  
        printf("video channel directory exist.\n");  
    }  
    else  
    {  
        printf("video channel directory is not exist, create directory.\n");  
		mkdir(video_channel_path, 0777);
		mkdir(video_day_path, 0777);
		mkdir(index_channel_path, 0777);
		mkdir(index_day_path, 0777);
		video_tmp_fd = open_tmp(video_tmp);
		index_tmp_fd = open_tmp(index_tmp);
		init_index_tmp(index_tmp_fd);
    } 	

	if ((access(video_day_path, F_OK)) != -1)  
    {  
        printf("video data directory exist.\n");  
    }  		
	else
	{
        printf("video data directory is not exist, create directory.\n");  
		mkdir(video_day_path, 0777);
		mkdir(index_day_path, 0777);	
		video_tmp_fd = open_tmp(video_tmp);
		index_tmp_fd = open_tmp(index_tmp);	
		init_index_tmp(index_tmp_fd);
	}
	
	if ((access(video_tmp, F_OK)) != -1)  
    {  
        printf("video tmp file exist.\n");  
    }  		
	else
	{
		if (cshmindex->type == P_FRAME_TYPE)
		{
			return 1;
		}
		else
		{
			video_tmp_fd = open_tmp(video_tmp);
			index_tmp_fd = open_tmp(index_tmp);	
			init_index_tmp(index_tmp_fd);			
		}
	}

	/* read_offset points next frame */
	shm_read_offset = cshmindex->offset + cshmindex->lenth + 1;

	if (cshmindex->type == I_FRAME_TYPE)
	{
		INDEX_INFO frecord;
		INDEX_INFO lrecord;
		INDEX_INFO crecord;	
		int ctime = 0;
		int ret = -1;
		char startbuf[7] = {0};
		char endbuf[7] = {0};
		char video_name_buf[64] = {0};
		char index_name_buf[64] = {0};

		ctime = convert_localtime_to_utc(frame);
		if (ctime == -1)
		{
			return -1;
		}
		ret = get_first_index_record(index_tmp_fd, &frecord);
		if (ret == -1)
		{
			return -1;
		}
		ret = get_last_index_record(index_tmp_fd, &lrecord);
		if (ret == -1)
		{
			return -1;
		}

		convert_utc_to_localtime(&frecord.time, startbuf);
		convert_utc_to_localtime(&lrecord.time, endbuf);
		sprintf(video_name_buf, "%s/%02d-%02d%02d%02d-%s-%s.h264", video_day_path, cshmindex->channel, 					cshmindex->time.year, cshmindex->time.month, cshmindex->time.day, startbuf, endbuf);
		sprintf(index_name_buf, "%s/%02d-%02d%02d%02d-%s-%s.index", index_day_path, cshmindex->channel, 	    			cshmindex->time.year, cshmindex->time.month, cshmindex->time.day, startbuf, endbuf);

		if ((lrecord.time != 0) && ((ctime-lrecord.time) > 2)) /* need to filter initialize record */
		{
			if ((access(video_tmp, F_OK))!=-1) 
			{
				close(video_tmp_fd);
			} 
			if ((access(index_tmp, F_OK))!=-1) 
			{
				close(index_tmp_fd);
			}
			/* rename video and index tmp file */ 
			rename(video_tmp, video_name_buf);
			rename(index_tmp, index_name_buf);
			
			/* create new video and index tmp file */
			video_tmp_fd = open_tmp(video_tmp);
			index_tmp_fd = open_tmp(index_tmp);	
			init_index_tmp(index_tmp_fd);

			/* storage one frame data and one index record */
			write(video_tmp_fd, frame, cshmindex->lenth); 
			get_current_index_record(index_tmp_fd, cshmindex, frame, &crecord);
			put_current_index_record(index_tmp_fd, &crecord);

			/* update share memory read_offset */
			memcpy(shared_memory_start, &shm_read_offset, sizeof(shm_read_offset));
					
		}
		else
		{
			write(video_tmp_fd, frame, cshmindex->lenth); 
			get_current_index_record(index_tmp_fd, cshmindex, frame, &crecord);
			put_current_index_record(index_tmp_fd, &crecord);	
			
			if ((lrecord.time != 0) && ((lrecord.time-frecord.time) >= 120)) /* need to filter initialize record */
			{
				close(video_tmp_fd);
				close(index_tmp_fd);
				rename(video_tmp, video_name_buf);
				rename(index_tmp, index_name_buf);	
				memcpy(shared_memory_start, &shm_read_offset, sizeof(shm_read_offset));
				return 1;			
			}	
			else
			{
				memcpy(shared_memory_start, &shm_read_offset, sizeof(shm_read_offset));
				return 1;
			}	
		}
	}
	else
	{
		write(video_tmp_fd, frame, cshmindex->lenth); 
		memcpy(shared_memory_start, &shm_read_offset, sizeof(shm_read_offset));
		return 1;
	}

	return 1;
}

void init_index_tmp(int index_tmp_fd)
{
	int ret = -1;
	INDEX_INFO init_record = {0, 0, 0};
	
	ret = write(index_tmp_fd, &init_record, sizeof(INDEX_INFO));
	if (ret == -1)
	{
		perror("initialize the index tmp file fail:");
		exit(EXIT_FAILURE);		
	}
	return;
}

int get_current_index_record(int index_tmp_fd, P4VEM_ShMIndex_t *cshmindex, FRAME_PACKET *cframe, INDEX_INFO *crecord)
{
	int ret = -1;
	int ctime = 0;
	INDEX_INFO lrecord;

	ret = get_last_index_record(index_tmp_fd, &lrecord);
	if (ret == -1)
	{
		return -1;		
	}

	ctime = convert_localtime_to_utc(cframe);
	if (ctime == -1)
	{
		return -1;
	}

	crecord->time = ctime;
	crecord->offset = lrecord.offset + lrecord.len;
	crecord->len = cshmindex->lenth;
	
	return 1;
}

int get_first_index_record(int index_tmp_fd, INDEX_INFO *frecord)
{
	int ret = -1;

	ret = lseek(index_tmp_fd, sizeof(INDEX_INFO), SEEK_SET); 
	if (ret == -1)
	{
		perror("lseek the start position of the index tmp file fail:");
		return -1;
	}

	ret = read(index_tmp_fd, frecord, sizeof(INDEX_INFO));
	if (ret == -1)
	{
		perror("read the first index record fail:");
		return -1;
	}
	else if (ret == 0)
	{
		printf("get_first_index_record  arrives end of file.\n");
	}

	return 1;
}

int put_current_index_record(int index_tmp_fd, INDEX_INFO *crecord)
{
	int ret = -1;

	ret = lseek(index_tmp_fd, 0, SEEK_END);
	if (ret == -1)
	{
		perror("lseek the index tmp file end position fail:");
		return -1;
	}

	ret = write(index_tmp_fd, crecord, sizeof(INDEX_INFO));
	if (ret == -1)
	{
		perror("write the current index record fail:");
		return -1;
	}	
	return 1;
}

int get_last_index_record(int index_tmp_fd, INDEX_INFO *lrecord)
{
	int ret = -1;

	ret = lseek(index_tmp_fd, -12, SEEK_END);
	if (ret == -1)
	{
		perror("lseek the last record position of the index tmp file fail:");
		return -1;
	}

	ret = read(index_tmp_fd, lrecord, sizeof(INDEX_INFO));
	if (ret == -1)
	{
		perror("read the last index record fail:");
		return -1;
	}
		
	return 1;
}

int fill_video_timeseg_array(const char* path, VIDEO_SEG_TIME timeseg[])
{
	return 0;
}

void sort_video_timeseg_array(VIDEO_SEG_TIME timeseg[], int left, int right)
{
	return;
}

int search_video_time_array(VIDEO_SEG_TIME timeseg[], VIDEO_SEG_TIME time)
{
	return 0;
}

int convert_localtime_to_utc(FRAME_PACKET *packet)
{
	if (packet == NULL)
	{
		fprintf(stderr, "convert_localtime_to_utc address error\n");
		exit(EXIT_FAILURE);
	}

	int ret = -1;
	struct tm info;

	info.tm_year = packet->rtc.stuRtcTime.cYear+2000 - 1900;
	info.tm_mon = packet->rtc.stuRtcTime.cMonth - 1;
	info.tm_mday = packet->rtc.stuRtcTime.cDay;
	info.tm_hour = packet->rtc.stuRtcTime.cHour;
	info.tm_min = packet->rtc.stuRtcTime.cMinute;
	info.tm_sec = packet->rtc.stuRtcTime.cSecond;
	info.tm_isdst = -1;

	ret = mktime(&info);
	if( ret == -1 )
	{
		perror("time convert error:");
		return -1;
	}

	return ret;	
}

void convert_utc_to_localtime(const unsigned int *time, char *ltime)
{
	if (time == NULL || ltime == NULL)
	{
		fprintf(stderr, "convert_utc_localtime address error\n");
		exit(EXIT_FAILURE);
	}

	const time_t *timep = (time_t *)time;
	struct tm *local = NULL;

	local = localtime(timep); 
	if (local == NULL)
	{
		perror("localtime error:");
		exit(EXIT_FAILURE);
	}
	
	sprintf(ltime,"%02d%02d%02d", local->tm_hour, local->tm_min, local->tm_sec);  
}


