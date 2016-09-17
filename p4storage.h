#ifndef _P4STORAGE_H
#define _P4STORAGE_H 1

#include "P4VEM_shm_index.h"

#define FRAME_SIZE 12 
#define PATH_LEN 64

#define SEARCH_CHANNEL_DATE 10
#define SEARCH_TIME 14

#define SHM_HEAD_SIZE 8

#define I_FRAME_TYPE	50
#define P_FRAME_TYPE	51

static int video_tmp_fd = -1;
static int index_tmp_fd = -1;
unsigned int shm_read_offset;

typedef struct _RMSTREAM_HEADER
{
	unsigned long IFrameType;	
	unsigned long IFrameLen:24; 
	unsigned long ISreamExam:8;
	unsigned long IExtendLen:24;
	unsigned long IExtendCount:8;
}RMSTREAM_HEADER;

typedef struct _RMS_INFOTYPE
{
	unsigned long LInfoType:8;
	unsigned long LInfoLength:24;
}RMS_INFOTYPE;

typedef struct _RMS_DATETIME
{
	unsigned char cYear;
	unsigned char cMonth;
	unsigned char cDay;
	unsigned char cHour;
	unsigned char cMinute;
	unsigned char cSecond;
	unsigned short usMilliSecond:10;
	unsigned short usWeek:3;
	unsigned short usReserved:2;
	unsigned short usMilliValidate:1;
}RMS_DATETIME;

typedef struct _RMFI2_RTCTIME
{
	RMS_INFOTYPE stuInfoTYpe;
	RMS_DATETIME stuRtcTime;
}RMFI2_RTCTIME;

typedef struct _RMFI2_VIDEOINFO
{
	RMS_INFOTYPE stuInfoTYpe;
	unsigned long IWidth:12;
	unsigned long IHeight:12;
	unsigned long IFPS:8;
}RMFI2_VIDEOINFO;

typedef struct _FRAME_PACKET
{
	RMSTREAM_HEADER head; /* packaged head;£º12byte */
	RMFI2_VIDEOINFO video; /* extended data£ºvideo info 8byte*/
	RMFI2_RTCTIME rtc; /* extended data£ºRTC 12byte */
	unsigned char frame[FRAME_SIZE]; /* storage video/audio array*/
}FRAME_PACKET;

typedef struct _INDEX_INFO
{
	unsigned int time;
	unsigned int offset;
	unsigned int len;
}INDEX_INFO;

typedef struct _VIDEO_SEG_TIME
{
	unsigned char start_time[7];
	unsigned char end_time[7];	
}VIDEO_SEG_TIME;

struct MEM_HEAD
{
	uint32_t read_offset; 
	uint32_t write_offset;
};

/* Get share memory setment. On success getshm() returns 
the address of the  attached  shared  memory segment; on error (void *) -1 is returned.*/
void* get_shm(int key, int size);

/* Open the file whose name is the string pointed to by path and associates a stream with it */
FILE *open_shm_index(const char *path);

/* Flushes the stream pointed to by fp and closes  the  underlying  file descriptor. 
On success returns 1; on error -1 is returned. */
int close_shm_index(FILE *indexfp);

/* If tmp file  exist of video or index, open it; Otherwise create tmp file ,then open it.
On success return fd; on error -1 is returned. */
int open_tmp(const char* tmpstring);

/* Get one record from shm index file, and compares with oldshmindex.
equal return 0; otherwise, -1 returned. */
int get_one_shm_index(FILE *indexfp, P4VEM_ShMIndex_t *oldshmindex/*in*/, P4VEM_ShMIndex_t *newshmindex/*in-out*/);

/* Get one frame data from share memory. 
On success return fd; on error -1 is returned. */
int get_one_frame(void *shared_memory_start/*in*/, void *shared_memory_end/*in*/, P4VEM_ShMIndex_t *cshmindex/*in*/, FRAME_PACKET *frame/*in-out*/);

/* Storage frame to disk or SD card.
On success returns 1; on error -1 is returned. */
int storage_one_frame(void *shared_memory_start/*in*/, P4VEM_ShMIndex_t *cshmindex/*in*/, FRAME_PACKET *frame/*in*/);

/* initialize the index tmp file. */
void init_index_tmp(int index_tmp_fd);

/* Generate current index record.
On success returns 1; on error -1 is returned. */
int get_current_index_record(int index_tmp_fd, P4VEM_ShMIndex_t *cshmindex/*in*/, FRAME_PACKET *cframe/*in*/, INDEX_INFO *crecord/*in-out*/);

/* Get first record from the index file.
On success returns 1; on error -1 is returned. */
int get_first_index_record(int index_tmp_fd, INDEX_INFO *frecord/*in-out*/);

/* Get current record from the index file.
On success returns 1; on error -1 is returned. */
int put_current_index_record(int index_tmp_fd, INDEX_INFO *crecord/*in-out*/);

/* Get last record from the index file.
On success returns 1; on error -1 is returned. */
int get_last_index_record(int index_tmp_fd, INDEX_INFO *lrecord/*in-out*/);

/* Get last front record from the index file.
On success returns 1; on error -1 is returned. */
int get_last_front_index_record(int index_tmp_fd, INDEX_INFO *lfrecord/*in-out*/);

/* Reads in at most one less than size characters from stream  and stores  
them  into  the buffer pointed to by time or date, Reading stops after an 
EOF or a newline, A '\0' is stored after the last character in the buffer */
void get_search_channel_date(char *channel_date_path/*in-out*/, int size, FILE *file);
void get_search_time(char *time/*in-out*/, int size, FILE *file);

/* Check video segment input date and time.
On true returns 1; on false -1 is returned.*/
int search_channel_date_check(char *channel_date_path/*in*/, int size);
int search_time_check(char *time/*in*/, int size);

/* Get the file name of all the video segment, then fill the VIDEO_SEG_TIME array.
On success returns a pointer to a VIDEO_SEG_TIME array; on error NULL is returned. */
VIDEO_SEG_TIME *fill_video_timeseg_array(const char* channel_date_path, int *video_seg_count/*in-out*/);

/* Free VIDEO_SEG_TIME pointer of fill_video_timeseg_array returns*/
void free_video_timeseg_array(VIDEO_SEG_TIME *timeseg/*in*/);

/* Order the array of video segments from small to large, timeseg pointer from 
fill_video_timeseg_array returns, left is 0, right is (video segment count - 1) */
void sort_video_timeseg_array(VIDEO_SEG_TIME timeseg[]/*in*/, int left, int right);

/* check and update search video time follow the timeseg array, timeseg pointer from 
fill_video_timeseg_array returns; update update_timeseg. */
int check_search_video_time(VIDEO_SEG_TIME timeseg[]/*in*/, int video_seg_count, const char *time/*in*/, VIDEO_SEG_TIME* update_timeseg/*in-out*/);

/* Search video segment, if have, output to stdout stream, then go back to terminal interface */
void output_search_video_info(const char* channel_date_path/*in*/, VIDEO_SEG_TIME timeseg[]/*in*/, 
											int video_seg_count, VIDEO_SEG_TIME *update_timeseg/*in*/);

/* Print Iframe information */
void print_iframe_info(const char* channel_date_path/*in*/, VIDEO_SEG_TIME *index_video_seg/*in*/, char * print_start_time/*in*/, char *print_end_time/*in*/);

/* Take an argument of data  type  calendar time which represents time_t.
On success returns time_t time; on error -1 is returned. */
int convert_localtime_to_utc(FRAME_PACKET *packet/*in*/);

/* Converts the calendar time  timep  to  broken-time  representation, */
void convert_utc_to_localtime(const unsigned int *time, char *ltime/*in-out*/);

#endif /* p4storage.h */


