#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include "p4storage.h"

int main()
{
  int running = 1;
  void *shared_memory_start = (void *)0;
  void *shared_memory_end = (void *)0;
  struct MEM_HEAD *head = NULL;
  FRAME_PACKET *packet = NULL;
  int shmid;
  unsigned int cnt = 0;

  unsigned int shm = sizeof(struct MEM_HEAD)+sizeof(FRAME_PACKET)*3;
  shmid = shmget((key_t)1234, shm, 0666 | IPC_CREAT);
  if (shmid == -1) 
  {
    fprintf(stderr, "shmget failed\n");
    exit(EXIT_FAILURE);
  }
  shared_memory_start = shmat(shmid, (void *)0, 0);
  if (shared_memory_start == (void *)-1) 
  {
    fprintf(stderr, "shmat failed\n");
    exit(EXIT_FAILURE);
  }
  shared_memory_end = shared_memory_start + shm;
  printf("Memory attached at %X\n", (int)shared_memory_start);
  printf("Memory end at %X\n", (int)shared_memory_end);
  
  head = (struct MEM_HEAD *)shared_memory_start;
  head->read_offset = 0;
  head->write_offset = 0;

  packet = (FRAME_PACKET *)(shared_memory_start + sizeof(struct MEM_HEAD));

  while(running--) 
  {
		/*******************************Frame No.1********************************/
		printf("%X\n", (int)packet);
		packet->head.IFrameType = 0x63643200;
  		packet->head.IFrameLen = 0x00000A;
  		packet->head.ISreamExam = 0xD6;
  		packet->head.IExtendLen = 0x000014;
  		packet->head.IExtendCount = 0x02;
		packet->video.stuInfoTYpe.LInfoType = 0x01;
		packet->video.stuInfoTYpe.LInfoLength = 0x000008;
		packet->video.IWidth = 0x280;
		packet->video.IHeight = 0x140;
		packet->video.IFPS = 0x1E;
  		packet->rtc.stuInfoTYpe.LInfoType = 0x02;
  		packet->rtc.stuInfoTYpe.LInfoLength = 0x00000C;
  		//packet->rtc.stuRtcTime = 0x0800072808100810;
  		packet->rtc.stuRtcTime.cYear = 0x10;
  		packet->rtc.stuRtcTime.cMonth = 0x08;
  		packet->rtc.stuRtcTime.cDay = 0x18;
  		packet->rtc.stuRtcTime.cHour = 0x0A;
  		packet->rtc.stuRtcTime.cMinute = 0x05;
  		packet->rtc.stuRtcTime.cSecond = 0x08;
  		packet->rtc.stuRtcTime.usMilliSecond = 0;
  		packet->rtc.stuRtcTime.usWeek = 2;
  		packet->rtc.stuRtcTime.usReserved = 0;
  		packet->rtc.stuRtcTime.usMilliValidate = 0;
  		strncpy(packet->frame, "abcdefghijkl", 12);

		P4VEM_ShMIndex_t shm_index1;
		shm_index1.type = 0x32;
		shm_index1.channel = 0x01;
		shm_index1.time.year = 0x10;
		shm_index1.time.month = 0x08;
		shm_index1.time.day = 0x18;
		shm_index1.time.hour = 0x0A;
		shm_index1.time.minute = 0x05;
		shm_index1.time.second = 0x08;
		shm_index1.offset = 0x08;
		shm_index1.lenth = 0x2C;

		int fd = open_tmp("./shmindex/shm.index");
		write(fd, &shm_index1, sizeof(P4VEM_ShMIndex_t));

		/*******************************Frame No.2********************************/
		packet = (FRAME_PACKET *)((char *)packet + sizeof(FRAME_PACKET));
		printf("%X\n", (int)packet);
		packet->head.IFrameType = 0x63643200;
  		packet->head.IFrameLen = 0x00000A;
  		packet->head.ISreamExam = 0xD6;
  		packet->head.IExtendLen = 0x000014;
  		packet->head.IExtendCount = 0x02;
		packet->video.stuInfoTYpe.LInfoType = 0x01;
		packet->video.stuInfoTYpe.LInfoLength = 0x000008;
		packet->video.IWidth = 0x280;
		packet->video.IHeight = 0x140;
		packet->video.IFPS = 0x1E;
  		packet->rtc.stuInfoTYpe.LInfoType = 0x02;
  		packet->rtc.stuInfoTYpe.LInfoLength = 0x00000C;
  		//packet->rtc.stuRtcTime = 0x0800072808100810;
  		packet->rtc.stuRtcTime.cYear = 0x10;
  		packet->rtc.stuRtcTime.cMonth = 0x08;
  		packet->rtc.stuRtcTime.cDay = 0x18;
  		packet->rtc.stuRtcTime.cHour = 0x0A;
  		packet->rtc.stuRtcTime.cMinute = 0x05;
  		packet->rtc.stuRtcTime.cSecond = 0x09;
  		packet->rtc.stuRtcTime.usMilliSecond = 0;
  		packet->rtc.stuRtcTime.usWeek = 2;
  		packet->rtc.stuRtcTime.usReserved = 0;
  		packet->rtc.stuRtcTime.usMilliValidate = 0;
  		strncpy(packet->frame, "abcdefghijkl", 12);

		P4VEM_ShMIndex_t shm_index2;
		shm_index2.type = 0x32;
		shm_index2.channel = 0x01;
		shm_index2.time.year = 0x10;
		shm_index2.time.month = 0x08;
		shm_index2.time.day = 0x18;
		shm_index2.time.hour = 0x0A;
		shm_index2.time.minute = 0x05;
		shm_index2.time.second = 0x09;
		shm_index2.offset = 0x08+44;
		shm_index2.lenth = 0x2C;

		write(fd, &shm_index2, sizeof(P4VEM_ShMIndex_t));

		/*******************************Frame No.3********************************/
		packet = (FRAME_PACKET *)((char *)packet + sizeof(FRAME_PACKET));
		printf("%X\n", (int)packet);
		packet->head.IFrameType = 0x63643200;
  		packet->head.IFrameLen = 0x00000A;
  		packet->head.ISreamExam = 0xD6;
  		packet->head.IExtendLen = 0x000014;
  		packet->head.IExtendCount = 0x02;
		packet->video.stuInfoTYpe.LInfoType = 0x01;
		packet->video.stuInfoTYpe.LInfoLength = 0x000008;
		packet->video.IWidth = 0x280;
		packet->video.IHeight = 0x140;
		packet->video.IFPS = 0x1E;
  		packet->rtc.stuInfoTYpe.LInfoType = 0x02;
  		packet->rtc.stuInfoTYpe.LInfoLength = 0x00000C;
  		//packet->rtc.stuRtcTime = 0x0800072808100810;
  		packet->rtc.stuRtcTime.cYear = 0x10;
  		packet->rtc.stuRtcTime.cMonth = 0x08;
  		packet->rtc.stuRtcTime.cDay = 0x18;
  		packet->rtc.stuRtcTime.cHour = 0x0A;
  		packet->rtc.stuRtcTime.cMinute = 0x05;
  		packet->rtc.stuRtcTime.cSecond = 0x0C;
  		packet->rtc.stuRtcTime.usMilliSecond = 0;
  		packet->rtc.stuRtcTime.usWeek = 2;
  		packet->rtc.stuRtcTime.usReserved = 0;
  		packet->rtc.stuRtcTime.usMilliValidate = 0;
  		strncpy(packet->frame, "abcdefghijkl", 12);

		P4VEM_ShMIndex_t shm_index3;
		shm_index3.type = 0x32;
		shm_index3.channel = 0x01;
		shm_index3.time.year = 0x10;
		shm_index3.time.month = 0x08;
		shm_index3.time.day = 0x18;
		shm_index3.time.hour = 0x0A;
		shm_index3.time.minute = 0x05;
		shm_index3.time.second = 0x0C;
		shm_index3.offset = 0x08+88;
		shm_index3.lenth = 0x2C;

		write(fd, &shm_index3, sizeof(P4VEM_ShMIndex_t));
  }


  if (shmdt(shared_memory_start) == -1) {
    fprintf(stderr, "shmdt failed\n");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
