#include "p4storage.h"

/* Open share memory index file. (This function has been abandoned.) */
FILE *open_shm_index(const char *path)
{
	if (path == NULL)
	{
		perror("open_shm_index string error:");
		p4_log(RUN_LOG, "open_shm_index string error.");
		exit(EXIT_FAILURE);		
	}

	FILE *indexfp = NULL;

	indexfp = fopen(path, "r");
	if (indexfp == NULL)
	{
		perror("fopen shm index file fail:");
		p4_log(RUN_LOG, "fopen shm index file fail.");
		exit(EXIT_FAILURE);
	}	
	
	return indexfp;
}

/* Close share memory index file. (This function has been abandoned.) */
int close_shm_index(FILE *indexfp)
{
	if (indexfp == NULL)
	{
		fprintf(stderr, "close_shm_index string error.\n");
		return -1;				
	}
	fclose(indexfp);
	
	return 1;
}

/* Given a pathname for a file, open_tmp() returns a file descriptor. */
int open_tmp(const char* tmpstring)
{
	if (tmpstring == NULL)
	{
		fprintf(stderr, "open_tmp string error.\n");
		return -1;				
	}

	int tmp = -1;

	tmp = open(tmpstring, O_RDWR | O_CREAT, 0666);
	if (tmp == -1)
	{
		perror("open_tm file fail.\n");
		p4_log(RUN_LOG, "open_tm file fail.");
		exit(EXIT_FAILURE);		
	}

	return tmp;
}

/* Read one index record from file. (This function has been abandoned.) */
int get_one_shm_index(FILE *indexfp, P4VEM_ShMIndex_t *oldshmindex, P4VEM_ShMIndex_t *newshmindex)
{
	if (indexfp == NULL || oldshmindex == NULL || newshmindex == NULL)
	{
		fprintf(stderr, "get_one_shm_index string error.\n");
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

/* Read one index record from share memory */
int get_one_index(void *shared_index_memory_start, P4VEM_ShMIndex_t *newshmindex)
{
	if (shared_index_memory_start == NULL || newshmindex == NULL)
	{
		fprintf(stderr, "get_one_index string error.\n");
		return -1;
	}

	unsigned int read_pos = 0;
	unsigned int write_pos = 0;
	P4VEM_ShMIndex_t *index = (P4VEM_ShMIndex_t *)(shared_index_memory_start + sizeof(P4VEM_ShM_IND_HEAD_t));

	memcpy(&read_pos, shared_index_memory_start, sizeof(unsigned int));
	memcpy(&write_pos, shared_index_memory_start + sizeof(unsigned int), sizeof(unsigned int));
	if (read_pos == write_pos)
	{
		#ifdef DEBUG
			printf("(read_pos == write_pos)::waiting shm index record update.\n");
		#endif
		return -1;
	}

	/* need have interval of one frame, let code module easy to judge, if no this condition,
	when read_pos equal write_pos, code module can't judge itself to be able to write. */
	if ((read_pos + 1) % SHM_INDEX_NUM == write_pos)
	{
		#ifdef DEBUG
			printf("(read_pos+1 == write_pos)::interval of one frame.\n");
		#endif
		return -1;
	}

	*newshmindex = index[read_pos];
	read_pos = (read_pos + 1) % SHM_INDEX_NUM;
	memcpy(shared_index_memory_start, &read_pos, sizeof(unsigned int));

	return 1;
}

/* Read one frame data from share memory according to frame index record. */
int get_one_frame(void *shared_memory_start, void *shared_memory_end, P4VEM_ShMIndex_t *cshmindex, FRAME_PACKET *frame)
{
	if (shared_memory_start == NULL || shared_memory_end == NULL || cshmindex == NULL || frame == NULL)
	{
		fprintf(stderr, "get_one_frame string error.\n");
		return -1;
	}	

	unsigned int read_offset = 0;
	unsigned int write_offset = 0;
	read_offset = *((unsigned int*)shared_memory_start);
	write_offset = *((unsigned int*)shared_memory_start + 1);
	
	if (read_offset == write_offset)
	{
		#ifdef DEBUG
			fprintf(stderr, "now, there is no data to read in the share memory, waitting for code module to write...\n");
		#endif
		return -1;
	}
	else if ((cshmindex->offset < write_offset) || ((cshmindex->offset > write_offset) && 
			((shared_memory_start + cshmindex->offset + cshmindex->lenth) <= shared_memory_end)))
	{
		if (cshmindex->type == P_FRAME_TYPE)
		{
			unsigned int tmp1 = FRAME_START_FLAG + sizeof(RMSTREAM_HEADER) + sizeof(RMFI2_VIDEOINFO);
			unsigned int tmp2 = sizeof(RMFI2_RTCTIME);
			unsigned int tmp3 = tmp1 + tmp2;
			memcpy(frame, shared_memory_start + cshmindex->offset + FRAME_START_FLAG, tmp1 - FRAME_START_FLAG);
			memcpy((void*)frame + tmp3, shared_memory_start + cshmindex->offset + tmp1, cshmindex->lenth - tmp1);	
		}
		else
		{
			memcpy(frame, shared_memory_start + cshmindex->offset + FRAME_START_FLAG, cshmindex->lenth - FRAME_START_FLAG);
		}		

		shm_read_offset = cshmindex->offset + cshmindex->lenth;
		return 1;
	}
	else if ((cshmindex->offset > write_offset) && 
			((shared_memory_start + cshmindex->offset + cshmindex->lenth) > shared_memory_end))
	{
		unsigned int to_end_len = shared_memory_end - (shared_memory_start + cshmindex->offset);
		unsigned int from_start_len = cshmindex->lenth - to_end_len;
		unsigned int SHM_HEAD_SIZE = sizeof(unsigned int) * 2;
		if (cshmindex->type == P_FRAME_TYPE)
		{
			unsigned int tmp1 = FRAME_START_FLAG + sizeof(RMSTREAM_HEADER) + sizeof(RMFI2_VIDEOINFO);
			unsigned int tmp2 = sizeof(RMFI2_RTCTIME);
			unsigned int tmp3 = tmp1 + tmp2;

			if (to_end_len >= tmp1)
			{
				unsigned int tmp = to_end_len - tmp1;
				memcpy(frame, shared_memory_start + cshmindex->offset + FRAME_START_FLAG, tmp1 - FRAME_START_FLAG);
				memcpy((void*)frame + tmp3, shared_memory_start + cshmindex->offset + tmp1, tmp);
				memcpy((void*)frame + tmp3 + tmp, shared_memory_start + SHM_HEAD_SIZE, from_start_len);
			}
			else
			{
				if (to_end_len >= FRAME_START_FLAG)
				{
					unsigned int tmp = tmp1 - to_end_len;
					memcpy(frame, shared_memory_start + cshmindex->offset + FRAME_START_FLAG, to_end_len - FRAME_START_FLAG);
					memcpy((void*)frame + (to_end_len - FRAME_START_FLAG), shared_memory_start + SHM_HEAD_SIZE, tmp);
					memcpy((void*)frame + tmp3, shared_memory_start + tmp + SHM_HEAD_SIZE, cshmindex->lenth - tmp1);
				}
				else
				{
					unsigned int tmp = from_start_len + (FRAME_START_FLAG - to_end_len);
					memcpy(frame, shared_memory_start + SHM_HEAD_SIZE + tmp, tmp1 - FRAME_START_FLAG);
					memcpy((void*)frame + tmp3, shared_memory_start + SHM_HEAD_SIZE + tmp + (tmp1 - FRAME_START_FLAG), cshmindex->lenth - tmp1);
				}
			}
		}
		else
		{
			if (to_end_len >= FRAME_START_FLAG)
			{
				memcpy(frame, shared_memory_start + cshmindex->offset + FRAME_START_FLAG, to_end_len - FRAME_START_FLAG);
				memcpy((void*)frame + (to_end_len - FRAME_START_FLAG), shared_memory_start + SHM_HEAD_SIZE, from_start_len);	
			}
			else
			{
				unsigned int tmp = from_start_len + (FRAME_START_FLAG - to_end_len);
				memcpy(frame, shared_memory_start + SHM_HEAD_SIZE + tmp, cshmindex->lenth - FRAME_START_FLAG);
			}
		}
	
		shm_read_offset = SHM_HEAD_SIZE + from_start_len;
		return 1;
	}	

	return 1;
}

/* Storage one frame data to tmp.h264 file, then write one index record to the tmp.index file. */
int storage_one_frame(void *shared_memory_start, P4VEM_ShMIndex_t *cshmindex, FRAME_PACKET *frame)
{
	if (shared_memory_start == NULL || cshmindex == NULL || frame == NULL)
	{
		fprintf(stderr, "storage_one_frame error.\n");
		return -1;				
	}

	int i = 0;
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

	if (cshmindex->type == I_FRAME_TYPE)
	{
		INDEX_INFO frecord = {0};
		INDEX_INFO lrecord = {0};
		INDEX_INFO crecord = {0};	

		int ctime = 0;
		char startbuf[7] = {0};
		char endbuf[7] = {0};
		char video_name_buf[PATH_LEN] = {0};
		char index_name_buf[PATH_LEN] = {0};

		if ((access(video_channel_path, F_OK)) != -1)  
    	{  
			#ifdef DEBUG
       			printf("video channel directory exist.\n");  
			#endif
    	}  
    	else  
   		{  
			#ifdef DEBUG
        		printf("video channel directory is not exist, create directory.\n");  
			#endif
			mkdir(video_channel_path, 0777);
			mkdir(video_day_path, 0777);
			mkdir(index_channel_path, 0777);
			mkdir(index_day_path, 0777);
			for (i = 0; i < CHANNEL_CNT; i++)
			{
				if (channel_tmp[i].channel == 0)
				{
					channel_tmp[i].channel = cshmindex->channel;
					channel_tmp[i].vt_fd = open_tmp(video_tmp);
					channel_tmp[i].it_fd = open_tmp(index_tmp);
					init_index_tmp(channel_tmp[i].it_fd);
					break;
				}
			}
   	 	} 
		if ((access(video_day_path, F_OK)) != -1)  
    	{  
			#ifdef DEBUG
        		printf("video data directory exist.\n");  
			#endif			
    	}  		
		else
		{
			#ifdef DEBUG
       			printf("video data directory is not exist, create directory.\n");  
			#endif
			mkdir(video_day_path, 0777);
			mkdir(index_day_path, 0777);	
			for (i = 0; i < CHANNEL_CNT; i++)
			{
				if (channel_tmp[i].channel == cshmindex->channel)
				{
					channel_tmp[i].vt_fd = open_tmp(video_tmp);
					channel_tmp[i].it_fd = open_tmp(index_tmp);
					init_index_tmp(channel_tmp[i].it_fd);
					break;
				}
			}
		}
		if ((access(video_tmp, F_OK)) != -1)  
    	{  
			#ifdef DEBUG
        		printf("video tmp file exist.\n");  
			#endif
				for (i = 0; i < CHANNEL_CNT; i++)
				{
					if (channel_tmp[i].channel == cshmindex->channel)
					{
						if((fcntl(channel_tmp[i].vt_fd, F_GETFL) == -1) && (fcntl(channel_tmp[i].it_fd, F_GETFL) == -1))
						{
							#ifdef DEBUG
							printf("%m::tmp file already closed, maybe restart this process. Now open tmp file again.\n");
							//printf("%s",strerror(errno)); // equal to printf("%m"); 
							#endif	
							channel_tmp[i].vt_fd = open_tmp(video_tmp);
							channel_tmp[i].it_fd = open_tmp(index_tmp);
							//return -1;
							break;
						}
					}
				}		
    	} 

		ctime = convert_localtime_to_utc(frame);
		if (ctime == -1)
		{
			return -1;
		}
		for (i = 0; i < CHANNEL_CNT; i++)
		{		
			if (channel_tmp[i].channel == cshmindex->channel)
			{
				int ret1 = -1;
				int ret2 = -1;

				ret1 = get_first_index_record(channel_tmp[i].it_fd, &frecord);
				ret2 = get_last_index_record(channel_tmp[i].it_fd, &lrecord);
				if (ret1 == -1 || ret2 == -1)
				{
					return -1;
				}
				break;
			}
		}		
		convert_utc_to_localtime(&frecord.time, startbuf);
		convert_utc_to_localtime(&lrecord.time, endbuf);
		sprintf(video_name_buf, "%s/%02d-%02d%02d%02d-%s-%s.h264", video_day_path, cshmindex->channel, 
					cshmindex->time.year, cshmindex->time.month, cshmindex->time.day, startbuf, endbuf);
		sprintf(index_name_buf, "%s/%02d-%02d%02d%02d-%s-%s.index", index_day_path, cshmindex->channel, 
					cshmindex->time.year, cshmindex->time.month, cshmindex->time.day, startbuf, endbuf);

		if (lrecord.time == 0)
		{
			#ifdef DEBUG
				printf("start storage the first Iframe.\n");
			#endif
			for (i = 0; i < CHANNEL_CNT; i++)
			{
				if (channel_tmp[i].channel == cshmindex->channel)
				{
					write(channel_tmp[i].vt_fd, frame, cshmindex->lenth - FRAME_START_FLAG); 
					get_current_index_record(channel_tmp[i].vt_fd, cshmindex, frame, &crecord);
					put_current_index_record(channel_tmp[i].it_fd, &crecord);
					break;
				}
			}	
			memcpy(shared_memory_start, &shm_read_offset, sizeof(shm_read_offset));
			return 1;
		}
		if ((ctime-lrecord.time) > 2) /* discontinuous */
		{
			if ((access(video_tmp, F_OK))!=-1) 
			{
				for (i = 0; i < CHANNEL_CNT; i++)
				{
					if (channel_tmp[i].channel == cshmindex->channel)
					{
						close(channel_tmp[i].vt_fd);
						close(channel_tmp[i].it_fd);

						rename(video_tmp, video_name_buf);
						rename(index_tmp, index_name_buf);

						channel_tmp[i].vt_fd = open_tmp(video_tmp);
						channel_tmp[i].it_fd = open_tmp(index_tmp);
						init_index_tmp(channel_tmp[i].it_fd);

						write(channel_tmp[i].vt_fd, frame, cshmindex->lenth - FRAME_START_FLAG); 
						get_current_index_record(channel_tmp[i].vt_fd, cshmindex, frame, &crecord);
						put_current_index_record(channel_tmp[i].it_fd, &crecord);
						break;
					}
				}			
			}			
			/* update share memory read_offset */
			memcpy(shared_memory_start, &shm_read_offset, sizeof(shm_read_offset));
			return 1;
		}
		else
		{
			INDEX_INFO lfrecord = {0};
			for (i = 0; i < CHANNEL_CNT; i++)
			{		
				if (channel_tmp[i].channel == cshmindex->channel)
				{
					int ret1 = -1;
					int ret2 = -1;
					int ret3 = -1;

					ret1 = get_last_front_index_record(channel_tmp[i].it_fd, &lfrecord);
					ret2 = get_first_index_record(channel_tmp[i].it_fd, &frecord);
					ret3 = get_last_index_record(channel_tmp[i].it_fd, &lrecord);
					if (ret1 == -1 || ret2 == -1 || ret3 == -1)
					{
						return -1;
					}

					convert_utc_to_localtime(&frecord.time, startbuf);
					convert_utc_to_localtime(&lrecord.time, endbuf);

					get_current_index_record(channel_tmp[i].vt_fd, cshmindex, frame, &crecord);
					break;
				}
			}
		 	if ((crecord.time - frecord.time) >= SEG_TIME) /* 123 / 133 */			
			{
				for (i = 0; i < CHANNEL_CNT; i++)
				{
					if (channel_tmp[i].channel == cshmindex->channel)
					{
						close(channel_tmp[i].vt_fd);
						close(channel_tmp[i].it_fd);

						rename(video_tmp, video_name_buf);
						rename(index_tmp, index_name_buf);	

						channel_tmp[i].vt_fd = open_tmp(video_tmp);
						channel_tmp[i].it_fd = open_tmp(index_tmp);
						init_index_tmp(channel_tmp[i].it_fd);

						write(channel_tmp[i].vt_fd, frame, cshmindex->lenth - FRAME_START_FLAG); 
						get_current_index_record(channel_tmp[i].vt_fd, cshmindex, frame, &crecord);
						put_current_index_record(channel_tmp[i].it_fd, &crecord);
						break;
					}
				}
				memcpy(shared_memory_start, &shm_read_offset, sizeof(shm_read_offset));
				return 1;	
			}
			else /* 122 */
			{
				for (i = 0; i < CHANNEL_CNT; i++)
				{
					if (channel_tmp[i].channel == cshmindex->channel)
					{
						write(channel_tmp[i].vt_fd, frame, cshmindex->lenth - FRAME_START_FLAG); 
						get_current_index_record(channel_tmp[i].vt_fd, cshmindex, frame, &crecord);
						put_current_index_record(channel_tmp[i].it_fd, &crecord);
						break;
					}
				}
				memcpy(shared_memory_start, &shm_read_offset, sizeof(shm_read_offset));
				return 1;
			}	
		}
	}
	else
	{
		unsigned tmp = sizeof(RMSTREAM_HEADER) + sizeof(RMFI2_VIDEOINFO);
		for (i = 0; i < CHANNEL_CNT; i++)
		{
			if (channel_tmp[i].channel == cshmindex->channel)
			{
				write(channel_tmp[i].vt_fd, frame, tmp); 
				write(channel_tmp[i].vt_fd, (void*)frame + tmp + sizeof(RMFI2_RTCTIME), cshmindex->lenth - tmp - FRAME_START_FLAG); 
				break;
			}
		}
		memcpy(shared_memory_start, &shm_read_offset, sizeof(shm_read_offset));		
		return 1;
	}

	return 1;
}

/* Initialize tmp.index file by one index record of all zero. */
void init_index_tmp(int index_tmp_fd)
{
	int ret = -1;
	INDEX_INFO init_record = {0, 0, 0};
	
	ret = write(index_tmp_fd, &init_record, sizeof(INDEX_INFO));
	if (ret == -1)
	{
		perror("initialize the index tmp file fail:");
		p4_log(RUN_LOG, "initialize the index tmp file fail.");
		exit(EXIT_FAILURE);		
	}

	return;
}

/*  Construct one index record. */
int get_current_index_record(int video_tmp_fd, P4VEM_ShMIndex_t *cshmindex, FRAME_PACKET *cframe, INDEX_INFO *crecord)
{
	if (cshmindex == NULL || cframe == NULL || crecord == NULL)
	{
		fprintf(stderr, "get_current_index_record string error.\n");
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
	crecord->offset = end_pos - (cshmindex->lenth - FRAME_START_FLAG);
	crecord->len = cshmindex->lenth - FRAME_START_FLAG;

	return 1;
}

/* Get the first index record in addition to initialized record. */
int get_first_index_record(int index_tmp_fd, INDEX_INFO *frecord)
{
	if (frecord == NULL)
	{
		fprintf(stderr, "get_first_index_record string error.\n");
		return -1;
	}

	int ret = -1;

	ret = lseek(index_tmp_fd, sizeof(INDEX_INFO), SEEK_SET); 
	if (ret == -1)
	{
		fprintf(stderr, "lseek the start position of the index tmp file fail.\n");
		return -1;
	}

	ret = read(index_tmp_fd, frecord, sizeof(INDEX_INFO));
	if (ret == -1)
	{
		fprintf(stderr, "read the first index record fail.\n");
		return -1;
	}
	else if (ret == 0)
	{
		#ifdef DEBUG
			printf("get_first_index_record  arrives end of file.\n");
		#endif
	}

	return 1;
}

/* Storage one index record to tmp.index file. */
int put_current_index_record(int index_tmp_fd, INDEX_INFO *crecord)
{
	if (crecord == NULL)
	{
		fprintf(stderr, "put_current_index_record string error.\n");
		return -1;
	}

	int ret = -1;

	ret = lseek(index_tmp_fd, 0, SEEK_END);
	if (ret == -1)
	{
		fprintf(stderr, "lseek the index tmp file end position fail:");
		return -1;
	}

	ret = write(index_tmp_fd, crecord, sizeof(INDEX_INFO));
	if (ret == -1)
	{
		fprintf(stderr, "write the current index record fail:");
		return -1;
	}	

	return 1;
}

/* Acquire the last record of the index file */
int get_last_index_record(int index_tmp_fd, INDEX_INFO *lrecord)
{
	if (lrecord == NULL)
	{
		fprintf(stderr, "get_last_index_record string error.\n");
		return;
	}

	int ret = -1;

	ret = lseek(index_tmp_fd, -sizeof(INDEX_INFO), SEEK_END);
	if (ret == -1)
	{
		fprintf(stderr, "lseek the last record position of the index tmp file fail:");
		return -1;
	}

	ret = read(index_tmp_fd, lrecord, sizeof(INDEX_INFO));
	if (ret == -1)
	{
		fprintf(stderr, "read the last index record fail:");
		return -1;
	}
		
	return 1;
}

/* Acquire the front record of the last record. */
int get_last_front_index_record(int index_tmp_fd, INDEX_INFO *lfrecord)
{
	if (lfrecord == NULL)
	{
		fprintf(stderr, "get_last_index_record string error.\n");
		return;
	}

	int ret = -1;

	ret = lseek(index_tmp_fd, -sizeof(INDEX_INFO)*2, SEEK_END);
	if (ret == -1)
	{
		fprintf(stderr, "lseek the last front record position of the index tmp file fail:");
		return -1;
	}

	ret = read(index_tmp_fd, lfrecord, sizeof(INDEX_INFO));
	if (ret == -1)
	{
		fprintf(stderr, "read the last front index record fail:");
		return -1;
	}
		
	return 1;	
}

/* Waiting to receive terminal input string of search channel and date. */
void get_search_channel_date(char *channel_date_path, int size, FILE *file)
{
	if (file == NULL)
	{
		fprintf(stderr, "get_search_channel_date string error.\n");
		return;
	}

	unsigned char tmp = 0;

	printf("Please input video search channel-date. Notice: dont't input any blank\nUsage: Channel-YearMonthDay\nSuch as:01-160816\nPlease start input channel and date:");
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
	while ((tmp = getc(file)) != '\n'); 
	
	return;
}

/* Waiting to receive terminal input string of search video segment time. */
void get_search_time(char *time, int size, FILE *file)
{
	if (file == NULL)
	{
		fprintf(stderr, "get_search_time string error.\n");
		return;
	}

	unsigned char tmp = 0;
	printf("Please input video search time. Notice: dont't input any blank\nUsage: HourMinuteSecond-HourMinuteSecond\nSuch as:084007-084016\nPlease start input time:");
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

	while ((tmp = getc(file)) != '\n');

	return;
}

/* Check the terminal input string of search channel and date. */
int search_channel_date_check(char *channel_date_path, int size)
{
	if (channel_date_path == NULL)
	{
		fprintf(stderr, "search_channel_date_check string error.\n");
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
				fprintf(stderr, L_RED "search channel and date input error: position at %d\nPlease input again.\n" NONE, cnt+1);
				return -1;
			}		
		}
		else if (cnt == 2)
		{
			if (channel_date_path[cnt] != '-')
			{
				fprintf(stderr, L_RED "search channel and date input error: position at %d\nPlease input again\n" NONE, cnt+1);
				return -1;
			}
		}
		else if (cnt <= 8 && cnt >= 3)
		{
			if (!(c <= '9'&& c >= '0'))
			{
				fprintf(stderr, L_RED "search channel and date input error: position at %d\nPlease input again\n" NONE, cnt+1);
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
		fprintf(stderr, L_RED "search date number error, Please input again.\n" NONE);
		return -1;
	}

	sprintf(channel, "./video/%c%c", channel_date_path[0], channel_date_path[1]);
	if ((access(channel, F_OK)) == -1)  
    {  
        printf(YELLOW "%c%c channel hasn't video record.\n\n" NONE, channel_date_path[0], channel_date_path[1]);  
		return -1;
    }  
	sprintf(date, "%s/%s", channel, channel_date_path+3);
	if ((access(date, F_OK)) == -1)  
    {  
        printf(YELLOW "%c%c channel hasn't this date video record.\n\n" NONE, channel_date_path[0], channel_date_path[1]);  
		return -1;
    }  

	return 1;
}

/* Check the terminal input string of search video segment time. */
int search_time_check(char *time, int size)
{
	if (time == NULL)
	{
		fprintf(stderr, "search_time_check string error.\n");
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
				fprintf(stderr, L_RED "search time input error: position at %d\nPlease input again\n" NONE, cnt+1);
				return -1;
			}		
		}
		else if (cnt == 6)
		{
			if (time[cnt] != '-')
			{
				fprintf(stderr, L_RED "search time input error: position at %d\nPlease input again\n" NONE, cnt+1);
				return -1;
			}
		}
		else if (cnt <= 12 && cnt >= 7)
		{
			if (!(c <= '9'&& c >= '0'))
			{
				fprintf(stderr, L_RED "search time input error: position at %d\nPlease input again\n" NONE, cnt+1);
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
	
	if (!((shour >= 0 && shour <= 24) && (ehour >= 0 && ehour <= 24) && 
		 (sminute >= 0 && sminute <= 59) && (eminute >= 0 && eminute <= 59) &&
		 (ssecond >= 0 && ssecond <= 59) && (esecond >= 0 && esecond <= 59)))
	{
		fprintf(stderr, L_RED "search time number error, Please input again\n" NONE);
		return -1;
	}

	if ((strcmp(start_time, end_time) > 0) || 
		((strcmp(start_time, end_time) > 0) && (strcmp(start_time + 3, end_time + 3) > 0)) ||
		((strcmp(start_time, end_time) > 0) && (strcmp(start_time + 3, end_time + 3) > 0) &&
		(strcmp(start_time + 6, end_time + 6) > 0)))
	{
		fprintf(stderr, L_RED "search video time: end time less than start time , input error.\n" NONE);	
		return -1;
	}

	return 1;
}

/* Get the heap pointer of video time segment array. */
VIDEO_SEG_TIME *fill_video_timeseg_array(const char* channel_date_path, int *video_seg_count)
{
	if (channel_date_path == NULL)
	{
		fprintf(stderr, "fill_video_timeseg_array string error.\n");
		return NULL;
	}

	int cnt = 0;
	DIR * dir = NULL;
	struct dirent * ptr = NULL;
	VIDEO_SEG_TIME *tmp = NULL;
	unsigned char channel[PATH_LEN] = {0};
	unsigned char date[PATH_LEN] = {0};
	
	/* check channel and date directory exist or no */
	sprintf(channel, "./video/%c%c", channel_date_path[0], channel_date_path[1]);
	if ((access(channel, F_OK)) == -1) 
	{
		fprintf(stderr, YELLOW "have't the channel video.\n" NONE);
		return NULL;
	} 

	sprintf(date, "./video/%c%c/%c%c%c%c%c%c", channel_date_path[0], channel_date_path[1], 
                         channel_date_path[3], channel_date_path[4], channel_date_path[5], 
                         channel_date_path[6], channel_date_path[7], channel_date_path[8]);

	if ((access(date, F_OK)) == -1) 
	{
		fprintf(stderr, YELLOW "have't the date video.\n" NONE);
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
		#ifdef DEBUG
			printf("have't video segment.\n");
		#endif
		return NULL;
	}
	tmp = (VIDEO_SEG_TIME *)malloc(sizeof(VIDEO_SEG_TIME) * cnt);
	memset(tmp, 0, sizeof(VIDEO_SEG_TIME) * cnt); /* need to clear, otherwise maybe generate an exception when repeat to search. */
	seekdir(dir, 0);
	cnt = 0;
	while((ptr = readdir(dir)) != NULL)
	{
		char *ret = strstr(ptr->d_name, channel_date_path);
		if (ret != 0)
		{
			strncpy((tmp+cnt)->start_time, (ptr->d_name) + 10, 6);
			strncpy((tmp+cnt)->end_time, (ptr->d_name) + 17, 6);
			cnt++;
		}
	}
	closedir(dir);
	*video_seg_count = cnt;

	return tmp;
}

/* Free heap memory. */
void free_video_timeseg_array(VIDEO_SEG_TIME *timeseg)
{
	if (timeseg == NULL)
	{
		fprintf(stderr, "free_video_timeseg_array heap pointer NULL.\n");
		return;
	}
	
	free(timeseg);

	return;
}

/* Use quick sort algorithm order the video time segment array. */
void sort_video_timeseg_array(VIDEO_SEG_TIME timeseg[], int left, int right)
{
	if (timeseg == NULL)
	{
		fprintf(stderr, "sort_video_timeseg_array pointer NULL.\n");
		return;
	}

	int i = 0;
	int j = 0;

	VIDEO_SEG_TIME tmp1 = {0};
	VIDEO_SEG_TIME tmp2 = {0};

    if(left > right) 
	{
		  return; 
	}

    memcpy(&tmp1, timeseg+left, sizeof(VIDEO_SEG_TIME));                            
    i = left; 
    j = right; 
    while(i != j) 
    { 
		while((strcmp(timeseg[j].start_time, tmp1.start_time) >= 0) && i < j) 
		{
			 j--; 
		}     
        while((strcmp(timeseg[i].start_time, tmp1.start_time) <= 0) && i < j) 
		{
			 i++; 
		}                                  
        if(i < j) 
        {
        	tmp2 = timeseg[i]; 
         	timeseg[i] = timeseg[j]; 
         	timeseg[j] = tmp2; 
        } 
    } 
    timeseg[left] = timeseg[i]; 
    timeseg[i] = tmp1; 
                             
    sort_video_timeseg_array(timeseg, left, i-1);
    sort_video_timeseg_array(timeseg, i+1, right);

	return;
}

/* Check the input video time is in the video segment array or not, then construct one 
VIDEO_SEG_TIME data by updated search time. */
int check_search_video_time(VIDEO_SEG_TIME timeseg[], int video_seg_count, const char *time, VIDEO_SEG_TIME* update_timeseg)
{
	if (timeseg == NULL)
	{
		fprintf(stderr, "check_search_video_time pointer error.\n");
		flag = 1; /* now only exist tmp.h264 video segment. */
		return -1;
	}

	/* before call check_search_video_time,  update_timeseg pointer elements must be zero,
	because just managers the top six bytes here */
	strncpy(update_timeseg->start_time, time, 6);
	strncpy(update_timeseg->end_time, time+7, 6);

	if ((strcmp(update_timeseg->end_time, timeseg[0].start_time) < 0))
	{
		fprintf(stderr, YELLOW "no corresponding video segment.\n" NONE);
		return -1;
	}

	if ((strcmp(update_timeseg->start_time, timeseg[video_seg_count-1].end_time) > 0))
	{
		#ifdef DEBUG
			printf("search video time don't in the already named video segment.\n");
		#endif
		flag = 1;
		return 0;
	}

	/* update video search time segment */
	if ((strcmp(update_timeseg->start_time, timeseg[0].start_time) < 0) &&
		 (strcmp(update_timeseg->end_time, timeseg[video_seg_count-1].end_time) > 0))
	{
		flag = 1;
		strncpy(update_timeseg->start_time, timeseg[0].start_time, 6);
		strncpy(update_timeseg->end_time, timeseg[video_seg_count-1].end_time, 6);
	}
	else if ((strcmp(update_timeseg->start_time, timeseg[0].start_time) < 0) &&
			(strcmp(update_timeseg->end_time, timeseg[video_seg_count-1].end_time) <= 0))
	{
		strncpy(update_timeseg->start_time, timeseg[0].start_time, 6);
	}
	else if ((strcmp(update_timeseg->end_time, timeseg[video_seg_count-1].end_time) > 0) &&
			(strcmp(update_timeseg->start_time, timeseg[0].start_time) >= 0))
	{
		flag = 1;
		strncpy(update_timeseg->end_time, timeseg[video_seg_count-1].end_time, 6);
	}

	return 1;
} 

/* Search tmp.h264 file for the time. */
void search_tmp_video_file(const char* channel_date_path, const char *time)
{
	if (channel_date_path == NULL || time == NULL)
	{
		fprintf(stderr, "search_tmp_video_file string error.\n");
		return;	
	}
	
	if (flag != 1)
	{
		#ifdef DEBUG
			printf("don't need search tmp.264.\n");
		#endif
		printf(YELLOW "Search completed.\n" NONE);
		return;
	}
	else
	{
		flag = 0;
		#ifdef DEBUG
			printf("finally, come to search tmp.264...\n");
		#endif
	}

	int ret = -1;
	int index_fd  = -1;
	int video_fd  = -1;
	VIDEO_SEG_TIME tmp_search = {0};
	INDEX_INFO first_tmp_index = {0};
	INDEX_INFO last_tmp_index = {0};
	unsigned char index_start[7] = {0};
	unsigned char index_end[7] = {0};
	
	unsigned char video[PATH_LEN] = {0};
	unsigned char index[PATH_LEN] = {0};

	sprintf(video, "./video/%c%c/%s/tmp.h264", channel_date_path[0], channel_date_path[1], 
																				channel_date_path+3);
	sprintf(index, "./index/%c%c/%s/tmp.index", channel_date_path[0], channel_date_path[1], 
																				channel_date_path+3);
	
	strncpy(tmp_search.end_time, time+7, 6); 
	strncpy(tmp_search.start_time, time, 6);

	if ((access(video, F_OK) != -1) && (access(index, F_OK) != -1)) 
	{
		index_fd = open_tmp(index);
		video_fd = open_tmp(video);
		ret = get_first_index_record(index_fd, &first_tmp_index);
		if (ret == -1)
		{
			fprintf(stderr, "search_tmp_video_file call get_first_index_record fail.\n");
			return;
		}
		ret = get_last_index_record(index_fd, &last_tmp_index);
		if (ret == -1)
		{
			fprintf(stderr, "search_tmp_video_file call get_last_index_record fail.\n");
			return;
		}
	}
	else
	{
		return;
	}

	convert_utc_to_localtime(&first_tmp_index.time, index_start);
	convert_utc_to_localtime(&last_tmp_index.time, index_end);

	/* tmp_search.start_time less than index_start or equal. */
	if ((strcmp(tmp_search.start_time, index_end) > 0) 
						 || (strcmp(tmp_search.end_time, index_start) < 0)) 
	{
		#ifdef DEBUG
			printf("tmp.h264 have't relative data, search completed.\n");
		#endif
		printf(YELLOW "Search completed.\n" NONE);
		return;
	}

	if (strcmp(tmp_search.end_time, index_end) <= 0)
	{
		/* output tmp.h264 from  index_start to tmp_search.end_time */
		print_tmp_video_info(video_fd, index_fd, tmp_search.end_time);
	}
	else if (strcmp(tmp_search.end_time, index_end) > 0)
	{
		/* output tmp.h264 from  index_start to index_end */
		print_tmp_video_info(video_fd, index_fd, index_end);
	}
	printf(YELLOW "Search completed.\n" NONE);

	return;
}

/* According to update_timeseg, begin to output relative video infomation. */
void output_search_video_info(const char* channel_date_path, VIDEO_SEG_TIME timeseg[], 
											int video_seg_count, VIDEO_SEG_TIME *update_timeseg)
{
	if (channel_date_path == NULL || timeseg == NULL || update_timeseg == NULL)
	{
		fprintf(stderr, "output_search_video_info string error.\n");
		return;
	}

	int i = 0;
	for (i = 0; i < video_seg_count; i++)
	{
		/* update_timeseg->start_time is in one video segment. */
		if ((strcmp(update_timeseg->start_time, timeseg[i].start_time) == 0) || 
			((strcmp(update_timeseg->start_time, timeseg[i].start_time) > 0) &&
		 	(strcmp(update_timeseg->start_time, timeseg[i].end_time) <= 0)))
		{
			/* update_timeseg is in one video segment. */
			if ((atoi(timeseg[i].end_time)-atoi(update_timeseg->end_time)) >= 0) 
			{
				#ifdef DEBUG
					printf("%s-%s\n", update_timeseg->start_time,  update_timeseg->end_time);
				#endif
				print_iframe_info(channel_date_path, &timeseg[i], update_timeseg->start_time, update_timeseg->end_time);	
				return;
			}
			else
			{
				#ifdef DEBUG
					printf("%s-%s\n", update_timeseg->start_time, timeseg[i].end_time);	
				#endif
				
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
					#ifdef DEBUG
						printf("%s-%s\n", timeseg[i].start_time, update_timeseg->end_time);	
					#endif
					print_iframe_info(channel_date_path, &timeseg[i], timeseg[i].start_time, update_timeseg->end_time);	
					return;
				}
				else
				{
					#ifdef DEBUG
						printf("%s-%s\n", timeseg[i].start_time, timeseg[i].end_time);	
					#endif	
					print_iframe_info(channel_date_path, &timeseg[i], timeseg[i].start_time, timeseg[i].end_time);	
					/* update search video time segment */
					if (strcmp(timeseg[i+1].start_time, update_timeseg->end_time) <= 0)
					{
						strncpy(update_timeseg->start_time, timeseg[i+1].start_time, 7);
						output_search_video_info(channel_date_path, timeseg, video_seg_count, update_timeseg);
					}
					i = video_seg_count; /* end current loop */
				}
			}
		}
	}

	return;
}

/* According to given start and end time, print video file frame infomation. */
void print_iframe_info(const char* channel_date_path, VIDEO_SEG_TIME *index_video_seg, char * print_start_time, char *print_end_time)
{
	if (channel_date_path == NULL || index_video_seg == NULL || print_start_time == NULL || print_end_time == NULL)
	{
		fprintf(stderr, "print_iframe_info string error.\n");
		return;
	}
	
	int ret = -1;
	int index_fd  = -1;
	int video_fd  = -1;
	INDEX_INFO tmp = {0};
	INDEX_INFO tmp1 = {0};
	FRAME_PACKET tmp2 = {0};
	unsigned char buf[7] = {0};
	unsigned char video[PATH_LEN] = {0};
	unsigned char index[PATH_LEN] = {0};

	memset(&tmp1, 0, sizeof(INDEX_INFO));
	memset(&tmp2, 0, sizeof(FRAME_PACKET));
	sprintf(video, "./video/%c%c/%s/%s-%s-%s.h264", channel_date_path[0], channel_date_path[1], 
		channel_date_path+3, channel_date_path, index_video_seg->start_time, index_video_seg->end_time);
	sprintf(index, "./index/%c%c/%s/%s-%s-%s.index", channel_date_path[0], channel_date_path[1], 
		channel_date_path+3, channel_date_path, index_video_seg->start_time, index_video_seg->end_time);

	index_fd = open_tmp(index);
	video_fd = open_tmp(video);
	lseek(index_fd, sizeof(INDEX_INFO), SEEK_SET);

	/* find print_start_time position */
	for (;;)
	{
		ret = read(index_fd, &tmp1, sizeof(INDEX_INFO));
		if (ret == -1)
		{
			fprintf(stderr, "read index file fail.\n");
			goto end;
		}
		else if (ret == 0)
		{
			printf("read index file EOF.\n");
			goto end;
		}
		convert_utc_to_localtime(&(tmp1.time), buf);
		if (strcmp(buf, print_start_time) != 0)
		{
			continue;
		}
		else
		{
			#ifdef DEBUG
				printf("already read print_start_time, begin output Iframe info.\n");
			#endif
			break;
		}
	}
	lseek(video_fd, tmp1.offset, SEEK_SET);
	read(video_fd, &tmp2, tmp1.len);
	print_valid_frame_data(&tmp2);

	while (strcmp(buf, print_end_time) != 0)
	{
		memset(&tmp1, 0, sizeof(INDEX_INFO));
		memset(&tmp2, 0, sizeof(FRAME_PACKET));
		ret = read(index_fd, &tmp1, sizeof(INDEX_INFO));
		if (ret == -1)
		{
			fprintf(stderr, "read index file fail.\n");
			goto end;
		}
		else if (ret == 0)
		{
			#ifdef DEBUG
				printf("read index file EOF, when occur the same RTC of the last two Iframes.\n");
			#endif
			goto end;
		}
		lseek(video_fd, tmp1.offset, SEEK_SET);
		read(video_fd, &tmp2, tmp1.len);
		print_valid_frame_data(&tmp2);
		convert_utc_to_localtime(&(tmp1.time), buf);
	}

	if (read(index_fd, &tmp, sizeof(INDEX_INFO)) > 0) /* occur two Iframe in one second. */
	{
		if (tmp.time == tmp1.time)
		{
			lseek(video_fd, tmp.offset, SEEK_SET);
			read(video_fd, &tmp2, tmp.len);
			print_valid_frame_data(&tmp2);
		}
		else
		{
			goto end;
		}
	}

end:
	close(index_fd);
	close(video_fd);

	return;
}

/* If tmp.h264 file include the relative frame, then print it. */
void print_tmp_video_info(int tmp_video_fd, int tmp_index_fd, char *print_end_time)
{
	if (print_end_time == NULL)
	{
		fprintf(stderr, "print_tmp_video_info string error.\n");
		return;
	}

	int ret = -1;
	INDEX_INFO tmp = {0};
	INDEX_INFO tmp1 = {0};
	FRAME_PACKET tmp2 = {0};
	unsigned char buf[7] = {0};
	
	lseek(tmp_index_fd, sizeof(INDEX_INFO), SEEK_SET);
	while (strcmp(buf, print_end_time) != 0)
	{
		ret = read(tmp_index_fd, &tmp1, sizeof(INDEX_INFO));
		if (ret == -1)
		{
			fprintf(stderr, "read tmp.index fail.\n");
			goto end;
		}
		lseek(tmp_video_fd, tmp1.offset, SEEK_SET);
		ret = read(tmp_video_fd, &tmp2, tmp1.len);
		if (ret == -1)
		{
			fprintf(stderr, "read tmp.h264 fail.\n");
			goto end;
		}
		print_valid_frame_data(&tmp2);
		convert_utc_to_localtime(&(tmp1.time), buf);
	}

	if (read(tmp_index_fd, &tmp, sizeof(INDEX_INFO)) > 0) /* occur two Iframe in one second. */
	{
		if (tmp.time == tmp1.time)
		{
			lseek(tmp_video_fd, tmp.offset, SEEK_SET);
			read(tmp_video_fd, &tmp2, tmp.len);
			print_valid_frame_data(&tmp2);
		}
		else
		{
			goto end;
		}
	}

end:
	close(tmp_video_fd);
	close(tmp_index_fd);

	return;
}

/* Print valid frame data. */
void  print_valid_frame_data(FRAME_PACKET *packet)
{
	int i = 0;
	printf(L_GREEN "time::" NONE);
	printf(UNDERLINE "%02d:%02d:%02d\n" NONE, packet->rtc.stuRtcTime.cHour, 
							packet->rtc.stuRtcTime.cMinute, packet->rtc.stuRtcTime.cSecond);
	printf(L_GREEN "data::" NONE);
	for (; i < PRINT_NUM; i++)
	{
		printf("%02x ", packet->frame[i]);
	}
	printf("...\n");
}

/* Given one frame packet, then convert localtime to utc. */
int convert_localtime_to_utc(FRAME_PACKET *packet)
{
	if (packet == NULL)
	{
		fprintf(stderr, "convert_localtime_to_utc address error.\n");
		exit(EXIT_FAILURE);
	}

	int ret = -1;
	struct tm info = {0};

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

/* Given utc, then convert utc to localtime. */
void convert_utc_to_localtime(const unsigned int *time, char *ltime)
{
	if (time == NULL || ltime == NULL)
	{
		fprintf(stderr, "convert_utc_localtime address error.\n");
		return;
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

/* List exist channel before search video segment. */
int list_channel(void)
{
	char cnt = 0;
	char *ret = NULL;
	DIR * dir = NULL;
	struct dirent * ptr = NULL;
	char video[8] = {0};
	char channel_0x[2] = {0};

	sprintf(video, "./video");
	sprintf(channel_0x, "0");

	dir = opendir(video);
	while((ptr = readdir(dir)) != NULL)
	{
		ret = strstr(ptr->d_name, channel_0x);
		if (ret != 0)
		{
			cnt++;
		}
	}
	if (cnt == 0)
	{
		printf("No channel video recording.\n");
		return cnt;
	}
	else
	{
		printf("*******exist the following channel video recording*******\n");
	}
	seekdir(dir, 0);
	while((ptr = readdir(dir)) != NULL)
	{
		ret = strstr(ptr->d_name, channel_0x);
		if (ret != 0)
		{
			printf(L_CYAN "%s\t" NONE, ptr->d_name);
		}
	}
	printf("\n");
	printf("*********exist the above channel video recording*********\n\n");
	closedir(dir);

	return cnt;
}

/* Define log function. */
void p4_log(int log_type, const char* format, ...)  
{  
    char wrlog[1024] = {0};  
    char buffer[1024] = {0};  
	char logpath[255] = {0};
	FILE* log_fp = NULL;
    va_list args;  
    va_start(args, format);  
    vsprintf(wrlog, format, args);  
    va_end(args);  
  
    time_t now = 0;;  
    time(&now);  
    struct tm *local = NULL;  
    local = localtime(&now);  

	#ifdef DEBUG
   	printf("%04d-%02d-%02d %02d:%02d:%02d %s\n", local->tm_year+1900, local->tm_mon, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, wrlog);
	#endif

    sprintf(buffer,"%04d-%02d-%02d %02d:%02d:%02d %s\n", local->tm_year+1900, local->tm_mon+1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, wrlog);  
	if (log_type == 1)
	{
		//sprintf(logpath, "./log/%02d%02d%02d-op.log", local->tm_year+1900-2000, local->tm_mon+1, local->tm_mday);
		sprintf(logpath, "./log/operate.log");
	}
    log_fp = fopen(logpath, "a+");  
	if (log_fp == NULL)
	{
		perror("open operate.log fail:");
		exit(EXIT_FAILURE);
	}
    fwrite(buffer, 1, strlen(buffer), log_fp);  
    fclose(log_fp);  
 
    return ;  
} 

void p4_terminal(void)
{
	int i = 0;
	int cnt = 0;
	int ret = -1;
	VIDEO_SEG_TIME * p = NULL;
	VIDEO_SEG_TIME update_timeseg = {0};
	char date[SEARCH_CHANNEL_DATE] = {0};
	char time[SEARCH_TIME] = {0};

	printf(L_PURPLE "*********************************************************\n" NONE);
	printf(L_PURPLE "*                                                       *\n" NONE);
	printf(L_PURPLE "*                  Welcome to GH4 group                 *\n" NONE);
	printf(L_PURPLE "*                       Enjoy it                        *\n" NONE);
	printf(L_PURPLE "*                                                       *\n" NONE);
	printf(L_PURPLE "*********************************************************\n" NONE);
	while(1)
	{
		for (;;)
		{
			list_channel();
			get_search_channel_date(date,sizeof(date), stdin);
			ret = search_channel_date_check(date, sizeof(date));
			if (ret == -1 )
			{
				continue;
			}
			else
			{
				printf("You input channel-date is:%s\n", date);
				break;
			}		
		}

		p = fill_video_timeseg_array(date, &cnt);
		sort_video_timeseg_array(p, 0, cnt-1);
		printf("\n***********exist the following video segments************\n");
		for (i=0; i<cnt; i++)
		{
			printf(L_CYAN "*\t\t\t%s-%s\t\t\t*\n" NONE, p[i].start_time, p[i].end_time);
		}
		printf("*************exist the above video segments**************\n\n");
		for (;;)
		{
			get_search_time(time,sizeof(time), stdin);
			int ret = search_time_check(time, sizeof(time));
			if (ret == -1 )
			{
				continue;
			}
			else
			{
				printf("You input time is:%s\n", time);
				break;
			}
		}

		#ifdef DEBUG
			printf("***************update_search_video_time**************\n");
		#endif
		memset(&update_timeseg, 0, sizeof(update_timeseg));
		ret = check_search_video_time(p, cnt, time, &update_timeseg);
		#ifdef DEBUG
			if (ret == 1)
			{
				printf("update time is::%s-%s\ntmp.h264 flag is::%d\n",update_timeseg.start_time, 
																   		update_timeseg.end_time, flag);
			}
			printf("***************output_search_video_info***************\n");
		#endif
		printf(YELLOW "\n*****************print I frame infomation****************\n" NONE);
		output_search_video_info(date, p, cnt, &update_timeseg);
		search_tmp_video_file(date, time);
		printf(YELLOW "*********************************************************\n\n" NONE);
		free_video_timeseg_array(p);
	}

	return;
}

void p4_video(key_t index_mem_key, size_t index_mem_size, key_t frame_mem_key, size_t frame_mem_size)
{
    int i = 0;
	int shmid1 = -1;
    int shmid2 = -1;
	unsigned int cnt = 0;
    FRAME_PACKET *packet = NULL;
    P4VEM_ShMIndex_t *index = NULL;
    P4VEM_ShM_DAT_HEAD_t *frame_head = NULL;
    P4VEM_ShM_IND_HEAD_t *index_head = NULL;
    void *shared_frame_memory_start = (void *)0;
    void *shared_frame_memory_end = (void *)0;
    void *shared_index_memory_start = (void *)0;
    void *shared_index_memory_end = (void *)0;

    shmid1 = shmget(index_mem_key, index_mem_size, 0666 | IPC_CREAT);
    if (shmid1 == -1) 
    {
		perror("index shmget failed:");
		p4_log(RUN_LOG, "index shmget failed.");
		exit(EXIT_FAILURE);
    }
    shared_index_memory_start = shmat(shmid1, (void *)0, 0);
    if (shared_index_memory_start == (void *)-1) 
    {
		perror("index shmat failed:");
		p4_log(RUN_LOG, "index shmat failed.");
		exit(EXIT_FAILURE);
    }
    shared_index_memory_end = shared_index_memory_start + index_mem_size;
	
	#ifdef DEBUG
		printf("***********************************************\n");
    	printf("index memory attached at %X\n", (int)shared_index_memory_start);
   		printf("index memory end at %X\n", (int)shared_index_memory_end);
	#endif  

    shmid2 = shmget(frame_mem_key, frame_mem_size, 0666 | IPC_CREAT);
    if (shmid2 == -1) 
    {
		fprintf(stderr, "frame shmget failed.\n");
		p4_log(RUN_LOG, "frame shmget failed.");
		exit(EXIT_FAILURE);
    }
    shared_frame_memory_start = shmat(shmid2, (void *)0, 0);
    if (shared_frame_memory_start == (void *)-1) 
    {
		fprintf(stderr, "frame shmat failed.\n");
		p4_log(RUN_LOG, "frame shmat failed.");
		exit(EXIT_FAILURE);
    }
    shared_frame_memory_end = shared_frame_memory_start + frame_mem_size;

	#ifdef DEBUG
   		printf("frame memory attached at %X\n", (int)shared_frame_memory_start);
    	printf("frame memory end at %X\n", (int)shared_frame_memory_end);
	#endif  

	index_head = (P4VEM_ShM_IND_HEAD_t *)shared_index_memory_start;
    index = (P4VEM_ShMIndex_t *)(shared_index_memory_start + sizeof(P4VEM_ShM_IND_HEAD_t));
    
	#ifdef DEBUG
		printf("***********************************************\n");
		printf("current write_pos::%d\n", index_head->write_pos); 
		P4VEM_ShMIndex_t shm_index = {0};
		for (i = 0; i < 6; i++)
		{
			shm_index = index[i];
			printf("type:%d channel:%d offset:%d lenth:%d\n", shm_index.type, shm_index.channel,
																	shm_index.offset, shm_index.lenth);
		}
		unsigned int read_pos = *((unsigned int*)shared_index_memory_start);
		unsigned int read_offset = *((unsigned int*)shared_frame_memory_start);	
		printf("start read_pos:%d\n", read_pos);
		printf("start read_offset:%d\n", read_offset);
		printf("***********************************************\n");
	#endif 
	for (;;)
	{
		int ret = -1;
		FRAME_PACKET packet1 = {0};
		P4VEM_ShMIndex_t newshmindex = {0};

		ret = get_one_index(shared_index_memory_start, &newshmindex);
		if (ret == -1)
		{
			usleep(10000);
			continue;
		}

		#ifdef DEBUG
			printf("%d-%d-%d %d:%d:%d %d\n", newshmindex.time.year, newshmindex.time.month, 
											 newshmindex.time.day , newshmindex.time.hour, 
											 newshmindex.time.minute, newshmindex.time.second, 
											 newshmindex.offset);
			unsigned int read_pos = *((unsigned int*)shared_index_memory_start);
			printf("read_pos:%d\n", read_pos);
		#endif 

		ret = get_one_frame(shared_frame_memory_start, shared_frame_memory_end, &newshmindex, &packet1);
		if (ret == -1)
		{
			usleep(10000);
			continue;
		}

		#ifdef DEBUG
			printf("%d-%d-%d %d:%d:%d\n", packet1.rtc.stuRtcTime.cYear, packet1.rtc.stuRtcTime.cMonth, 
										  packet1.rtc.stuRtcTime.cDay, packet1.rtc.stuRtcTime.cHour, 
			      						  packet1.rtc.stuRtcTime.cMinute,packet1.rtc.stuRtcTime.cSecond);
			printf("frametype::%x\n", (unsigned int)packet1.head.IFrameType);
		#endif 

		storage_one_frame(shared_frame_memory_start, &newshmindex, &packet1);
		{
		#ifdef DEBUG
			unsigned int read_offset = *((unsigned int*)shared_frame_memory_start);	
			printf("read_offset:%d\n", read_offset);
			printf("***********************************************\n");
		#endif 
		}
	}
	
	return;
}


