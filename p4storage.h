/**********************************************************************
 Copyright(C),2016-2018,www.streamax.com
 File name: p4stotage.h
 Author: ZhaoKun
 Version: V3.3.3
 Date: 2016-9-17
 Desrciption: This file is used to declare some function of storage module 
			  and these functions are called by CenterServer module.
 Others: Given a serial of custom types for communicate to code module. 
***********************************************************************/

#ifndef _P4STORAGE_H
#define _P4STORAGE_H 1

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
#include <stdarg.h>
#include <stdint.h>
#include <sys/socket.h>
#include <pthread.h>

#include "P4VEM_shm_index.h"
#include "p4show.h"

//#define DEBUG 1

#define FRAME_SIZE 1024*30
#define SHM_INDEX_NUM 1024
#define FRAME_START_FLAG	5

#define CHANNEL_CNT		8
#define SEG_TIME	10
#define PRINT_NUM	50	/* print valid Iframe data count. */

#define PATH_LEN	255
#define SEARCH_CHANNEL_DATE		10
#define SEARCH_TIME		14

#define I_FRAME_TYPE	50
#define P_FRAME_TYPE	51

#define STORAGE_RUN_LOG		1	/* Storage Module */
#define CSM_RUN_LOG			2	/* CenterServer Module*/
#define CM_RUN_LOG			3	/* Code  Module */
#define DM_RUN_LOG			4	/* Device Module */

typedef struct _RMSTREAM_HEADER
{
	uint32_t IFrameType;	
	uint32_t IFrameLen:24; 
	uint32_t ISreamExam:8;
	uint32_t IExtendLen:24;
	uint32_t IExtendCount:8;
}RMSTREAM_HEADER;

typedef struct _RMS_INFOTYPE
{
	uint32_t LInfoType:8;
	uint32_t LInfoLength:24;
}RMS_INFOTYPE;

typedef struct _RMS_DATETIME
{
	uint8_t cYear;
	uint8_t cMonth;
	uint8_t cDay;
	uint8_t cHour;
	uint8_t cMinute;
	uint8_t cSecond;
	uint16_t usMilliSecond:10;
	uint16_t usWeek:3;
	uint16_t usReserved:2;
	uint16_t usMilliValidate:1;
}RMS_DATETIME;

typedef struct _RMFI2_RTCTIME
{
	RMS_INFOTYPE stuInfoTYpe;
	RMS_DATETIME stuRtcTime;
}RMFI2_RTCTIME;

typedef struct _RMFI2_VIDEOINFO
{
	RMS_INFOTYPE stuInfoTYpe;
	uint32_t IWidth:12;
	uint32_t IHeight:12;
	uint32_t IFPS:8;
}RMFI2_VIDEOINFO;

typedef struct _FRAME_PACKET
{
	RMSTREAM_HEADER head; /* packaged head;£º12byte */
	RMFI2_VIDEOINFO video; /* extended data£ºvideo info 8byte*/
	RMFI2_RTCTIME rtc; /* extended data£ºRTC 12byte */
	uint8_t frame[FRAME_SIZE]; /* storage video/audio array*/
}FRAME_PACKET;

typedef struct _INDEX_INFO
{
	uint32_t time;
	uint32_t offset;
	uint32_t len;
}INDEX_INFO;

typedef struct _VIDEO_SEG_TIME
{
	uint8_t start_time[7];
	uint8_t end_time[7];	
}VIDEO_SEG_TIME;

typedef struct _tmp_fd
{
	uint8_t channel;
	int32_t vt_fd;
	int32_t it_fd;
}tmp_fd;

/* Open the file whose name is the string pointed to by path and associates a stream with it */
extern FILE *open_shm_index(const char *path);

/* Flushes the stream pointed to by fp and closes  the  underlying  file descriptor. 
On success returns 1; on error -1 is returned. */
extern int close_shm_index(FILE *indexfp);

/* If tmp file  exist of video or index, open it; Otherwise create tmp file ,then open it.
On success return fd; on error -1 is returned. */
extern int open_tmp(const char* tmpstring);

/* Get one record from shm index file, and compares with oldshmindex.
equal return 0; otherwise, -1 returned. */
extern int get_one_shm_index(FILE *indexfp, P4VEM_ShMIndex_t *oldshmindex/*in*/, P4VEM_ShMIndex_t *newshmindex/*in-out*/);

/* Acquire one index record from share memory.
On success returns 1; on error -1 is returned. */
extern int get_one_index(void *shared_index_memory_start/*in*/, P4VEM_ShMIndex_t *newshmindex/*in-out*/);

/* Get one frame data from share memory. 
On success return fd; on error -1 is returned. */
extern int get_one_frame(void *shared_memory_start/*in*/, void *shared_memory_end/*in*/, P4VEM_ShMIndex_t *cshmindex/*in*/, FRAME_PACKET *frame/*in-out*/);

/* Storage frame to disk or SD card.
On success returns 1; on error -1 is returned. */
extern int storage_one_frame(void *shared_memory_start/*in*/, P4VEM_ShMIndex_t *cshmindex/*in*/, FRAME_PACKET *frame/*in*/);

/* initialize the index tmp file. */
extern void init_index_tmp(int index_tmp_fd);

/* Generate current index record.
On success returns 1; on error -1 is returned. */
int get_current_index_record(int index_tmp_fd, P4VEM_ShMIndex_t *cshmindex/*in*/, FRAME_PACKET *cframe/*in*/, INDEX_INFO *crecord/*in-out*/);

/* Get first record from the index file.
On success returns 1; on error -1 is returned. */
extern int get_first_index_record(int index_tmp_fd, INDEX_INFO *frecord/*in-out*/);

/* Get current record from the index file.
On success returns 1; on error -1 is returned. */
extern int put_current_index_record(int index_tmp_fd, INDEX_INFO *crecord/*in-out*/);

/* Get last record from the index file.
On success returns 1; on error -1 is returned. */
extern int get_last_index_record(int index_tmp_fd, INDEX_INFO *lrecord/*in-out*/);

/* Get last front record from the index file.
On success returns 1; on error -1 is returned. */
extern int get_last_front_index_record(int index_tmp_fd, INDEX_INFO *lfrecord/*in-out*/);

/* Reads in at most one less than size characters from stream  and stores  
them  into  the buffer pointed to by time or date, Reading stops after an 
EOF or a newline, A '\0' is stored after the last character in the buffer */
extern void get_search_channel_date(char *channel_date_path/*in-out*/, int size, FILE *file);
extern void get_search_time(char *time/*in-out*/, int size, FILE *file);

/* Check video segment input date and time.
On true returns 1; on false -1 is returned.*/
extern int search_channel_date_check(char *channel_date_path/*in*/, int size);
extern int search_time_check(char *time/*in*/, int size);

/* Get the file name of all the video segment, then fill the VIDEO_SEG_TIME array.
On success returns a pointer to a VIDEO_SEG_TIME array; on error NULL is returned. */
VIDEO_SEG_TIME *fill_video_timeseg_array(const char* channel_date_path, int *video_seg_count/*in-out*/);

/* Free VIDEO_SEG_TIME pointer of fill_video_timeseg_array returns*/
extern void free_video_timeseg_array(VIDEO_SEG_TIME *timeseg/*in*/);

/* Order the array of video segments from small to large, timeseg pointer from 
fill_video_timeseg_array returns, left is 0, right is (video segment count - 1) */
extern void sort_video_timeseg_array(VIDEO_SEG_TIME timeseg[]/*in*/, int left, int right);

/* check and update search video time follow the timeseg array, timeseg pointer from 
fill_video_timeseg_array returns; update update_timeseg. On true returns 1; on false -1 is returned. */
extern int check_search_video_time(VIDEO_SEG_TIME timeseg[]/*in*/, int video_seg_count, const char *time/*in*/, VIDEO_SEG_TIME* update_timeseg/*in-out*/);

/* Achieve to search the tmp video file. */
//void search_tmp_video_file(const char* channel_date_path/*in*/, VIDEO_SEG_TIME timeseg[], int video_seg_count, const char *time, int *flag);
extern void search_tmp_video_file(const char* channel_date_path/*in*/, const char *time);

/* Search video segment, if have, output to stdout stream, then go back to terminal interface */
extern void output_search_video_info(const char* channel_date_path/*in*/, VIDEO_SEG_TIME timeseg[]/*in*/, 
											int video_seg_count, VIDEO_SEG_TIME *update_timeseg/*in*/);

/* Print Iframe information */
extern void print_iframe_info(const char* channel_date_path/*in*/, VIDEO_SEG_TIME *index_video_seg/*in*/, char * print_start_time/*in*/, char *print_end_time/*in*/);

/* Print tmp.h264 information */
extern void print_tmp_video_info(int tmp_video_fd, int tmp_index_fd, char *print_end_time/*in*/);

/* Print valid frame data. */
extern void print_valid_frame_data(FRAME_PACKET *packet);

/* Take an argument of data  type  calendar time which represents time_t.
On success returns time_t time; on error -1 is returned. */
extern int convert_localtime_to_utc(FRAME_PACKET *packet/*in*/);

/* Converts the calendar time  timep  to  broken-time  representation, */
extern void convert_utc_to_localtime(const unsigned int *time, char *ltime/*in-out*/);

/* List exist channel of video directory. 
if exist channel video record ,returns channel count; or 0 returned*/
extern int list_channel(void);

/* Define log function. */
extern void p4_log(int log_type, const char* format, ...);

/* Terminal interaction of video search. */
extern void p4_terminal(void);

/* Video storage. */
extern void p4_video(key_t index_mem_key, size_t index_mem_size, key_t frame_mem_key, size_t frame_mem_size);

extern void p4_heart(void);

extern void p4_log_collect(void);

/* Initialize storage module, send register message to CenterServer. */
extern void p4_storage_init(void);

#endif /* p4storage.h */


