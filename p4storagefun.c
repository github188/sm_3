#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
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
    	perror("shmget failed:");
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
		perror("open_shm_index string error:");
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

int close_shm_index(FILE *indexfp)
{
	if (indexfp == NULL)
	{
		fprintf(stderr, "close_shm_index string error\n");
		return -1;				
	}
	fclose(indexfp);
	
	return 1;
}

int open_tmp(const char* tmpstring)
{
	if (tmpstring == NULL)
	{
		fprintf(stderr, "open_tmp string error\n");
		return -1;				
	}

	int tmp = -1;

	tmp = open(tmpstring, O_RDWR|O_CREAT, 0666);
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
		fprintf(stderr, "get_one_shm_index string error\n");
		return -1;
	}	

	unsigned char cnt = 0;

	cnt = fread(newshmindex, sizeof(P4VEM_ShMIndex_t), 1, indexfp);	
	if (cnt == 0)
	{
		fprintf(stderr, "fread an error  occurs, or the end-of-file is reached.\n");
		return -1;
	}

	if (newshmindex->offset == oldshmindex->offset)
	{
		return -1;
	}
	
	memcpy(oldshmindex, newshmindex, sizeof(P4VEM_ShMIndex_t));

	return 0;
}

int get_one_frame(void *shared_memory_start, P4VEM_ShMIndex_t *cshmindex, FRAME_PACKET *frame)
{
	if (shared_memory_start == NULL || cshmindex == NULL || frame == NULL)
	{
		fprintf(stderr, "get_one_frame string error\n");
		return -1;
	}	
	
	if (cshmindex->type == P_FRAME_TYPE)
	{
		unsigned int tmp1 = sizeof(RMSTREAM_HEADER) + sizeof(RMFI2_VIDEOINFO);
		unsigned int tmp2 = sizeof(RMFI2_RTCTIME);
		unsigned int tmp3 = tmp1 + tmp2;
		memcpy(frame, shared_memory_start + cshmindex->offset, tmp1);
		memcpy((void*)frame + tmp3, shared_memory_start + cshmindex->offset + tmp1, cshmindex->lenth - tmp1);
		//printf("Pframe:%s\n", frame->frame);
	}
	else
	{
		memcpy(frame, shared_memory_start + cshmindex->offset, cshmindex->lenth);	
		//printf("Iframe:%04x\n", (unsigned int)frame->head.IFrameType);
	}

	return 1;
}

/* Storage_one_frame function only serve one channel for two specified file description, 
 video_tmp_fd and index_tmp_fd*/
int storage_one_frame(void *shared_memory_start, P4VEM_ShMIndex_t *cshmindex, FRAME_PACKET *frame)
{
	if (shared_memory_start == NULL || cshmindex == NULL || frame == NULL)
	{
		fprintf(stderr, "storage_one_frame error\n");
		return -1;				
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
		
	/* read_offset points next frame */
	shm_read_offset = cshmindex->offset + cshmindex->lenth;

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
		sprintf(video_name_buf, "%s/%02d-%02d%02d%02d-%s-%s.h264", video_day_path, cshmindex->channel, 
					cshmindex->time.year, cshmindex->time.month, cshmindex->time.day, startbuf, endbuf);
		sprintf(index_name_buf, "%s/%02d-%02d%02d%02d-%s-%s.index", index_day_path, cshmindex->channel, 
					cshmindex->time.year, cshmindex->time.month, cshmindex->time.day, startbuf, endbuf);

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
			get_current_index_record(video_tmp_fd, cshmindex, frame, &crecord);
			put_current_index_record(index_tmp_fd, &crecord);

			/* update share memory read_offset */
			memcpy(shared_memory_start, &shm_read_offset, sizeof(shm_read_offset));
					
		}
		else
		{
			INDEX_INFO lfrecord;
			ret = get_last_index_record(index_tmp_fd, &lfrecord);
			if (ret == -1)
			{
				return -1;
			}

			write(video_tmp_fd, frame, cshmindex->lenth); 
			get_current_index_record(video_tmp_fd, cshmindex, frame, &crecord);
			put_current_index_record(index_tmp_fd, &crecord);	

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

			if ((lrecord.time != 0) && ((lrecord.time - frecord.time + 1) >= 3)) /* need to filter initialize record */
			{
				convert_utc_to_localtime(&lrecord.time, endbuf);
				sprintf(video_name_buf, "%s/%02d-%02d%02d%02d-%s-%s.h264",
 										video_day_path, cshmindex->channel, cshmindex->time.year, 
										cshmindex->time.month, cshmindex->time.day, startbuf, endbuf);
				sprintf(index_name_buf, "%s/%02d-%02d%02d%02d-%s-%s.index", 
										index_day_path, cshmindex->channel, cshmindex->time.year,
 										cshmindex->time.month, cshmindex->time.day, startbuf, endbuf);
				close(video_tmp_fd);
				close(index_tmp_fd);
				rename(video_tmp, video_name_buf);
				rename(index_tmp, index_name_buf);	
				memcpy(shared_memory_start, &shm_read_offset, sizeof(shm_read_offset));
				return 1;			
			}
			else if ((lrecord.time != 0) && ((lrecord.time - frecord.time + 2) >= 3) &&
																	(lfrecord.time == lrecord.time))
			{
				lrecord.time = lrecord.time + 1; /* last index time equal front of last index time */
				convert_utc_to_localtime(&lrecord.time, endbuf);
				sprintf(video_name_buf, "%s/%02d-%02d%02d%02d-%s-%s.h264",
 										video_day_path, cshmindex->channel, cshmindex->time.year, 
										cshmindex->time.month, cshmindex->time.day, startbuf, endbuf);
				sprintf(index_name_buf, "%s/%02d-%02d%02d%02d-%s-%s.index", 
										index_day_path, cshmindex->channel, cshmindex->time.year,
 										cshmindex->time.month, cshmindex->time.day, startbuf, endbuf);
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
		if(fcntl(video_tmp_fd, F_GETFL))  
		{
			printf("%m::video segment tmp file already closed, discard final Pframe\n");
			return -1;
		}
		unsigned tmp = sizeof(RMSTREAM_HEADER)+sizeof(RMFI2_VIDEOINFO);
		write(video_tmp_fd, frame, tmp); 
		write(video_tmp_fd, (void*)frame + tmp + sizeof(RMFI2_RTCTIME), cshmindex->lenth - tmp); 
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

int get_current_index_record(int video_tmp_fd, P4VEM_ShMIndex_t *cshmindex, FRAME_PACKET *cframe, INDEX_INFO *crecord)
{
	if (cshmindex == NULL || cframe == NULL || crecord == NULL)
	{
		fprintf(stderr, "get_current_index_record string error\n");
		return -1;
	}	

	unsigned int end_pos = 0;
	int ctime = 0;
	

	end_pos = lseek(video_tmp_fd, 0, SEEK_END);
	if (end_pos == -1)
	{
		return -1;		
	}

	ctime = convert_localtime_to_utc(cframe);
	if (ctime == -1)
	{
		return -1;
	}

	crecord->time = ctime;
	crecord->offset = end_pos - cshmindex->lenth;
	crecord->len = cshmindex->lenth;
	
	return 1;
}

int get_first_index_record(int index_tmp_fd, INDEX_INFO *frecord)
{
	if (frecord == NULL)
	{
		fprintf(stderr, "get_first_index_record string error\n");
		return -1;
	}

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
	if (crecord == NULL)
	{
		fprintf(stderr, "put_current_index_record string error\n");
		return -1;
	}

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
	if (lrecord == NULL)
	{
		fprintf(stderr, "get_last_index_record string error\n");
		return;
	}

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

void get_search_channel_date(char *channel_date_path, int size, FILE *file)
{
	if (file == NULL)
	{
		fprintf(stderr, "get_search_channel_date string error\n");
		return;
	}

	unsigned char c = 0;

	printf("Please input video search channel and date. Notice: dont't input any blank\nUsage: Channel-YearMonthDay\nSuch as:01-160825\nPlease start input channel and date:");
	fflush(stdout);

	fgets(channel_date_path, size, file);
	if (strlen(channel_date_path) < (size -1 )) /* '\n' is read. */
	{
		return; 
	}
	else if ((strlen(channel_date_path) == (size - 1)) && channel_date_path[size - 2] == '\n')
	{
		return;
	}

	/* discard stdin remaining character, avoid to affect next input*/
	while ((c = getc(file)) != '\n'); 
	
	return;
}

void get_search_time(char *time, int size, FILE *file)
{
	if (file == NULL)
	{
		fprintf(stderr, "get_search_time string error\n");
		return;
	}

	unsigned char c = 0;
	printf("Please input video search time. Notice: dont't input any blank\nUsage: HourMinuteSecond-HourMinuteSecond\nSuch as:154606-154630\nPlease start input time:");
	fflush(stdout);

	fgets(time, size, file);
	if (strlen(time) < (size-1))
	{
		return; 
	}
	else if ((strlen(time) == (size-1)) && time[size-2] == '\n')
	{
		return;
	}

	while ((c = getc(file)) != '\n');

	return;
}

int search_channel_date_check(char *channel_date_path, int size)
{
	if (channel_date_path == NULL)
	{
		fprintf(stderr, "search_channel_date_check string error\n");
		return -1;
	}

	unsigned char c = 0;
	unsigned char cnt = 0;
	unsigned char len = size-1;

	unsigned char channel[PATH_LEN] = {0};
	unsigned char date[PATH_LEN] = {0};
	unsigned char channel_date[9] = {0};

	unsigned char year = -1;
	unsigned char month = -1;
	unsigned char day = -1;	

	while(cnt < len)
	{
		c = channel_date_path[cnt];
		if (cnt >= 0 && cnt <= 1)
		{
			if (!(c <= '9'&& c >= '0'))
			{
				fprintf(stderr, "Search channel and date input error: position at %d\nPlease input again.\n", cnt+1);
				return -1;
			}		
		}
		else if (cnt == 2)
		{
			if (channel_date_path[cnt] != '-')
			{
				fprintf(stderr, "Search channel and date input error: position at %d\nPlease input again\n", cnt+1);
				return -1;
			}
		}
		else if (cnt <= 8 && cnt >= 3)
		{
			if (!(c <= '9'&& c >= '0'))
			{
				fprintf(stderr, "Search channel and date input error: position at %d\nPlease input again\n", cnt+1);
				return -1;
			}					
		}
		cnt++;
	}

	sprintf(channel_date, "%c%c%c%c%c%c%c%c", channel_date_path[3], channel_date_path[4], '\0',
		channel_date_path[5], channel_date_path[6], '\0', channel_date_path[7], channel_date_path[8]);	
	year = atoi(channel_date);
	month = atoi(channel_date + 3);
	day = atoi(channel_date + 6);	
	
	if(!((year >= 16) && (month <= 12 && month >= 1) && (day <= 31 && day >= 1)))
	{
		fprintf(stderr, "Search date number error, Please input again\n");
		return -1;
	}

	sprintf(channel, "./video/%c%c", channel_date_path[0], channel_date_path[1]);
	if ((access(channel, F_OK)) == -1)  
    {  
        printf("have't channel:%s video exist.\n", channel);  
		return -1;
    }  
	sprintf(date, "%s/%s", channel, channel_date_path+3);
	if ((access(date, F_OK)) == -1)  
    {  
        printf("channel:%s have't date:%s video exist.\n", channel, date);  
		return -1;
    }  

	return 1;
}

int search_time_check(char *time, int size)
{
	if (time == NULL)
	{
		fprintf(stderr, "search_time_check string error\n");
		return -1;
	}

	unsigned char c = 0;
	unsigned char cnt = 0;
	unsigned char len = size-1;

	unsigned char start_time[9] = {0};
	unsigned char end_time[9] = {0};

	unsigned char shour = -1;
	unsigned char sminute = -1;
	unsigned char ssecond = -1;
	unsigned char ehour = -1;
	unsigned char eminute = -1;
	unsigned char esecond = -1;			

	while(cnt < len)
	{
		c = time[cnt];
		if (cnt >= 0 && cnt <= 5)
		{
			if (!(c <= '9'&& c >= '0'))
			{
				fprintf(stderr, "Search time input error: position at %d\nPlease input again\n", cnt+1);
				return -1;
			}		
		}
		else if (cnt == 6)
		{
			if (time[cnt] != '-')
			{
				fprintf(stderr, "Search time input error: position at %d\nPlease input again\n", cnt+1);
				return -1;
			}
		}
		else if (cnt <= 12 && cnt >= 7)
		{
			if (!(c <= '9'&& c >= '0'))
			{
				fprintf(stderr, "Search time input error: position at %d\nPlease input again\n", cnt+1);
				return -1;
			}					
		}
		cnt++;
	}

	sprintf(start_time, "%c%c%c%c%c%c%c%c", time[0], time[1], '\0',
									        time[2], time[3], '\0', time[4], time[5]);
	sprintf(end_time,   "%c%c%c%c%c%c%c%c", time[7], time[8], '\0',
									        time[9], time[10], '\0', time[11], time[12]);

	shour = atoi(start_time);
	sminute = atoi(start_time + 3);
	ssecond = atoi(start_time + 6);
	ehour = atoi(end_time);
	eminute = atoi(end_time + 3);
	esecond = atoi(end_time + 6);
	
	if (!((shour>=0 && shour<=24) && (ehour>=0 && ehour<=24) && 
		 (sminute>=0 && sminute<=59) && (eminute>=0 && eminute<=59) &&
		 (ssecond>=0 && ssecond<=59) && (esecond>=0 && esecond<=59)))
	{
		fprintf(stderr, "Search time number error, Please input again\n");
		return -1;
	}

	if ((strcmp(start_time, end_time) > 0) || (strcmp(start_time+3, end_time+3) > 0) ||
		(strcmp(start_time+6, end_time+6) > 0))
	{
		fprintf(stderr, "search video time: end time less than start time , input error\n");	
		return -1;
	}

	return 1;
}

/* get the heap pointer of video time segment. */
VIDEO_SEG_TIME *fill_video_timeseg_array(const char* channel_date_path, int *video_seg_count)
{
	if (channel_date_path == NULL)
	{
		fprintf(stderr, "fill_video_timeseg_array string error\n");
		return NULL;
	}

	int i = 0;
	int cnt = 0;
	DIR * dir = NULL;
	struct dirent * ptr = NULL;
	VIDEO_SEG_TIME *tmp = NULL;
	unsigned char channel[PATH_LEN] = {0};
	unsigned char date[PATH_LEN] = {0};
	
	/* check channel and date directory is exist or no */
	sprintf(channel, "./video/%c%c", channel_date_path[0], channel_date_path[1]);
	if ((access(channel, F_OK)) == -1) 
	{
		fprintf(stderr, "have't the channel video\n");
		return NULL;
	} 

	sprintf(date, "./video/%c%c/%c%c%c%c%c%c", channel_date_path[0], channel_date_path[1], 
                         channel_date_path[3], channel_date_path[4], channel_date_path[5], 
                         channel_date_path[6], channel_date_path[7], channel_date_path[8]);

	if ((access(date, F_OK)) == -1) 
	{
		fprintf(stderr, "have't the date video\n");
		return NULL;
	} 	

	/* fill video segment name time array */
	dir = opendir(date);
	while((ptr = readdir(dir)) != NULL)
	{
		char *ret = strstr(ptr->d_name, channel_date_path);
		if (ret != 0)
		{
			cnt++;
		}
	}
	if (cnt == 0)
	{
		printf("have't video segment\n");
		return NULL;
	}
	tmp = (VIDEO_SEG_TIME *)malloc(sizeof(VIDEO_SEG_TIME) * cnt);
	seekdir(dir, 0);
	cnt = 0;
	while((ptr = readdir(dir)) != NULL)
	{
		char *ret = strstr(ptr->d_name, channel_date_path);
		if (ret != 0)
		{
			strncpy((tmp+cnt)->start_time, (ptr->d_name)+10, 6);
			strncpy((tmp+cnt)->end_time, (ptr->d_name)+17, 6);
			cnt++;
		}
	}
	closedir(dir);
	*video_seg_count = cnt;

	return tmp;
}

/* use quick sort algorithm order the video time segment. */
void sort_video_timeseg_array(VIDEO_SEG_TIME timeseg[], int left, int right)
{
	if (timeseg == NULL)
	{
		fprintf(stderr, "sort_video_timeseg_array pointer error\n");
		return;
	}

	int i = 0;
	int j = 0;
	VIDEO_SEG_TIME t;
	VIDEO_SEG_TIME tmp;

    if(left > right) 
	{
		  return; 
	}

    memcpy(&tmp, timeseg+left, sizeof(VIDEO_SEG_TIME));                            
    i = left; 
    j = right; 
    while(i != j) 
    { 
		while((strcmp(timeseg[j].start_time, tmp.start_time) >= 0) && i < j) 
		{
			 j--; 
		}     
        while((strcmp(timeseg[i].start_time, tmp.start_time) <= 0) && i < j) 
		{
			 i++; 
		}                                  
        if(i < j) 
        {
        	t = timeseg[i]; 
         	timeseg[i] = timeseg[j]; 
         	timeseg[j] = t; 
        } 
    } 
    timeseg[left] = timeseg[i]; 
    timeseg[i] = tmp; 
                             
    sort_video_timeseg_array(timeseg, left, i-1);
    sort_video_timeseg_array(timeseg, i+1, right);

	return;
}

int check_search_video_time(VIDEO_SEG_TIME timeseg[], int video_seg_count, const char *time, VIDEO_SEG_TIME* update_timeseg)
{
	if (timeseg == NULL)
	{
		fprintf(stderr, "check_search_video_time pointer error\n");
		return -1;
	}

	/* before call check_search_video_time,  update_timeseg pointer elements must be zero,
	because just managers the top six bytes here */
	strncpy(update_timeseg->start_time, time, 6);
	strncpy(update_timeseg->end_time, time+7, 6);

	if ((strcmp(update_timeseg->start_time, timeseg[video_seg_count-1].end_time) > 0) ||
		 strcmp(update_timeseg->end_time, timeseg[0].start_time) < 0)
	{
		fprintf(stderr, "have't check_search_video_time\n");
		return -1;
	}

	/* update video search time segment */
	if ((strcmp(update_timeseg->start_time, timeseg[0].start_time) < 0) &&
		 strcmp(update_timeseg->end_time, timeseg[video_seg_count-1].end_time) > 0)
	{
		strncpy(update_timeseg->start_time, timeseg[0].start_time, 6);
		strncpy(update_timeseg->end_time, timeseg[video_seg_count-1].end_time, 6);
	}
	else if (strcmp(update_timeseg->start_time, timeseg[0].start_time) < 0)	
	{
		strncpy(update_timeseg->start_time, timeseg[0].start_time, 6);
	}
	else if (strcmp(update_timeseg->end_time, timeseg[video_seg_count-1].end_time) > 0)
	{
		strncpy(update_timeseg->end_time, timeseg[video_seg_count-1].end_time, 6);
	}

	return 1;
} 

void output_search_video_info(const char* channel_date_path, VIDEO_SEG_TIME timeseg[], 
											int video_seg_count, VIDEO_SEG_TIME *update_timeseg)
{
	if (channel_date_path == NULL || timeseg == NULL || update_timeseg == NULL)
	{
		fprintf(stderr, "output_search_video_info string error\n");
		return;
	}

	int i = 0;
	for (i = 0; i < video_seg_count; i++)
	{
		if ((strcmp(update_timeseg->start_time, timeseg[i].start_time) == 0) || 
			((strcmp(update_timeseg->start_time, timeseg[i].start_time) > 0) &&
		 	(strcmp(update_timeseg->start_time, timeseg[i].end_time) <= 0)))
		{
			if ((atoi(timeseg[i].end_time)-atoi(timeseg[i].start_time)) >= 
				(atoi(update_timeseg->end_time)-atoi(update_timeseg->start_time))) 
			{
				printf("%s-%s\n", update_timeseg->start_time,  update_timeseg->end_time);
				print_iframe_info(channel_date_path, &timeseg[i], update_timeseg->start_time, update_timeseg->end_time);		
				return;
			}
			else
			{
				printf("%s-%s\n", update_timeseg->start_time, timeseg[i].end_time);	
				print_iframe_info(channel_date_path, &timeseg[i], update_timeseg->start_time, timeseg[i].end_time);	
				/* update search video time segment */
				if (strcmp(timeseg[i+1].start_time, update_timeseg->end_time) <= 0)
				{
					strncpy(update_timeseg->start_time, timeseg[i+1].start_time, 7);
					output_search_video_info(channel_date_path, timeseg, video_seg_count, update_timeseg);
				}
				i = video_seg_count; /* end current loop */
			}
		}
		else
		{
			/* get the first video time segment more than update_timeseg */
			if ((strcmp(timeseg[i].start_time, update_timeseg->start_time) > 0) &&
				(strcmp(timeseg[i].start_time, update_timeseg->end_time) <= 0))
			{
				if ((atoi(timeseg[i].end_time)-atoi(timeseg[i].start_time)) >= 
					(atoi(update_timeseg->end_time)-atoi(timeseg[i].start_time))) 
				{
					printf("%s-%s\n", timeseg[i].start_time, update_timeseg->end_time);	
					print_iframe_info(channel_date_path, &timeseg[i], timeseg[i].start_time, update_timeseg->end_time);		
					return;
				}
				else
				{
					printf("%s-%s\n", timeseg[i].start_time, timeseg[i].end_time);		
					print_iframe_info(channel_date_path, &timeseg[i], timeseg[i].start_time, timeseg[i].end_time);	

					/* update search video time segment */
					if (strcmp(timeseg[i+1].start_time, update_timeseg->end_time) <= 0)
					{
						strncpy(update_timeseg->start_time, timeseg[i+1].start_time, 7);
						output_search_video_info(channel_date_path, timeseg, video_seg_count, 
																					update_timeseg);
					}
					i = video_seg_count; /* end current loop */
				}
			}
		}
	}
}

void print_iframe_info(const char* channel_date_path, VIDEO_SEG_TIME *index_video_seg, char * print_start_time, char *print_end_time)
{
	if (index_video_seg == NULL || print_start_time == NULL || print_end_time == NULL)
	{
		fprintf(stderr, "print_iframe_info string error\n");
		return;
	}
	
	int ret = -1;
	int index_fd  = -1;
	int video_fd  = -1;
	INDEX_INFO tmp1;
	FRAME_PACKET tmp2;
	unsigned char buf[7] = {0};
	unsigned char video[PATH_LEN] = {0};
	unsigned char index[PATH_LEN] = {0};

	memset(&tmp1, 0, sizeof(INDEX_INFO));
	memset(&tmp2, 0, sizeof(FRAME_PACKET));
	sprintf(video, "./video/%c%c/%s/%s-%s-%s.h264", channel_date_path[0], channel_date_path[1], channel_date_path+3, channel_date_path, index_video_seg->start_time, index_video_seg->end_time);
	sprintf(index, "./index/%c%c/%s/%s-%s-%s.index", channel_date_path[0], channel_date_path[1], channel_date_path+3, channel_date_path, index_video_seg->start_time, index_video_seg->end_time);

	index_fd = open_tmp(index);
	video_fd = open_tmp(video);
	lseek(index_fd, sizeof(INDEX_INFO), SEEK_SET);
	/* find print_start_time position */
	for (;;)
	{
		ret = read(index_fd, &tmp1, sizeof(INDEX_INFO));
		if (ret == -1)
		{
			fprintf(stderr, "read index file fail\n");
			goto end;
		}
		else if (ret == 0)
		{
			printf("read index file EOF\n");
			goto end;
		}
		convert_utc_to_localtime(&(tmp1.time), buf);
		if (strcmp(buf, print_start_time) != 0)
		{
			continue;
		}
		else
		{
			printf("already read print_start_time, begin output Iframe info\n");
			break;
		}
	}
	lseek(video_fd, tmp1.offset, SEEK_SET);
	read(video_fd, &tmp2, tmp1.len);
	printf("already read Iframe info: %#04X:%s\n", (unsigned int)tmp2.head.IFrameType, tmp2.frame);
	while (strcmp(buf, print_end_time) != 0)
	{
		memset(&tmp1, 0, sizeof(INDEX_INFO));
		memset(&tmp2, 0, sizeof(FRAME_PACKET));
		ret = read(index_fd, &tmp1, sizeof(INDEX_INFO));
		if (ret == -1)
		{
			fprintf(stderr, "read index file fail\n");
			goto end;
		}
		else if (ret == 0)
		{
			printf("read index file EOF\n");
			goto end;
		}
		lseek(video_fd, tmp1.offset, SEEK_SET);
		read(video_fd, &tmp2, tmp1.len);
		printf("already read Iframe info: %#04X:%s\n", (unsigned int)tmp2.head.IFrameType, tmp2.frame);
		convert_utc_to_localtime(&(tmp1.time), buf);
	}

end:
	close(index_fd);
	close(video_fd);
	return;
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

	return;
}


