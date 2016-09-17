#ifndef _P4STORAGE_H
#define _P4STORAGE_H 1

#include "P4VEM_shm_index.h"

//#define FRAME_SIZE 128*1024 
#define FRAME_SIZE 12 
#define PATH_LEN 64

#define I_FRAME_TYPE	50
#define P_FRAME_TYPE	51

static int video_tmp_fd = -1;
static int index_tmp_fd = -1;

typedef unsigned char BYTE;

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
	BYTE cYear;
	BYTE cMonth;
	BYTE cDay;
	BYTE cHour;
	BYTE cMinute;
	BYTE cSecond;
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
	unsigned int time;	
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

/* Flushes the stream pointed to by fp and closes  the  underlying  file descriptor. */
void close_shm_index(FILE *indexfp);

/* If tmp file  exist of video or index, open it; Otherwise create tmp file ,then open it.
On success return fd; on error -1 is returned. */
int open_tmp(const char* tmpstring);

/* Get one record from shm index file, and compares with oldshmindex.
equal return 0; otherwise, -1 returned. */
int get_one_shm_index(FILE *indexfp, P4VEM_ShMIndex_t *oldshmindex/*in*/, P4VEM_ShMIndex_t *newshmindex/*in-out*/);

/* Get one frame data from share memory. */
void get_one_frame(void *shared_memory_start, P4VEM_ShMIndex_t *cshmindex/*in*/, FRAME_PACKET *frame/*in-out*/);

/* Storage frame to disk or SD card.
On success returns 1; on error -1 is returned. */
int storage_one_frame(void *shared_memory_start, P4VEM_ShMIndex_t *cshmindex/*in*/, FRAME_PACKET *frame/*in*/);

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

/* Get the file name of all the video segment, then fill the VIDEO_SEG_TIME array;
On success returns 1; on error -1 is returned. */
int fill_video_timeseg_array(const char* path, VIDEO_SEG_TIME timeseg[]);

/* Order the array of video segments from small to large */
void sort_video_timeseg_array(VIDEO_SEG_TIME timeseg[], int left, int right);

/* Search video time in the timeseg array
On success returns the array subscript; on error -1 is returned. */
int search_video_time_array(VIDEO_SEG_TIME timeseg[], VIDEO_SEG_TIME time);

/* Take an argument of data  type  calendar time which represents time_t.
On success returns time_t time; on error -1 is returned. */
int convert_localtime_to_utc(FRAME_PACKET *packet);

/* Converts the calendar time  timep  to  broken-time  representation, */
void convert_utc_to_localtime(const unsigned int *time/*in*/, char *ltime/*in-out*/);

#endif /* p4storage.h */


