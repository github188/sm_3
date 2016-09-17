#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include "p4storage.h"

unsigned int shm_write_offset;
unsigned int shm_write_pos;

int main()
{
  int running = 1;
  void *shared_frame_memory_start = (void *)0;
  void *shared_frame_memory_end = (void *)0;
  void *shared_index_memory_start = (void *)0;
  void *shared_index_memory_end = (void *)0;
  P4VEM_ShM_DAT_HEAD_t *frame_head = NULL;
  P4VEM_ShM_IND_HEAD_t *index_head = NULL;
  FRAME_PACKET *packet = NULL;
  P4VEM_ShMIndex_t *index = NULL;
  int shmid1;
  int shmid2;
  unsigned int cnt = 0;

  unsigned int shm1 = SHM_IND_TOTAL_SIZE;
  shmid1 = shmget((key_t)5678, shm1, 0666 | IPC_CREAT);
  if (shmid1 == -1) 
  {
    fprintf(stderr, "shmget failed\n");
    exit(EXIT_FAILURE);
  }
  shared_index_memory_start = shmat(shmid1, (void *)0, 0);
  if (shared_index_memory_start == (void *)-1) 
  {
    fprintf(stderr, "shmat failed\n");
    exit(EXIT_FAILURE);
  }
  shared_index_memory_end = shared_index_memory_start + shm1;
  printf("Index memory attached at %X\n", (int)shared_index_memory_start);
  printf("Index memory end at %X\n", (int)shared_index_memory_end);
  
  unsigned int shm2 = SHM_DAT_TOTAL_SIZE;
  shmid2 = shmget((key_t)1234, shm2, 0666 | IPC_CREAT);
  if (shmid2 == -1) 
  {
    fprintf(stderr, "shmget failed\n");
    exit(EXIT_FAILURE);
  }
  shared_frame_memory_start = shmat(shmid2, (void *)0, 0);
  if (shared_frame_memory_start == (void *)-1) 
  {
    fprintf(stderr, "shmat failed\n");
    exit(EXIT_FAILURE);
  }
  shared_frame_memory_end = shared_frame_memory_start + shm2;
  printf("Frame memory attached at %X\n", (int)shared_frame_memory_start);
  printf("Frame memory end at %X\n", (int)shared_frame_memory_end);


  frame_head = (P4VEM_ShM_DAT_HEAD_t *)shared_frame_memory_start;
  frame_head->read_offset = 8;
  frame_head->write_offset = 8;
  packet = (FRAME_PACKET *)(shared_frame_memory_start + sizeof(P4VEM_ShM_DAT_HEAD_t));
  printf("packet: %#X\n", (unsigned int)packet);
  printf("shm_write_offset::%d\n", frame_head->write_offset);

  index_head = (P4VEM_ShM_IND_HEAD_t *)shared_index_memory_start;
  index_head->read_pos = 0;
  index_head->write_pos = 0;
  index = ( P4VEM_ShMIndex_t *)(shared_index_memory_start + sizeof(P4VEM_ShM_IND_HEAD_t));
  printf("index: %#X\n", (unsigned int)index);
  printf("shm_write_pos::%d\n", index_head->write_pos);  
  printf("===============================================\n");
  while(running--) 
  {
		//*******************************IFrame No.1********************************
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

		index[0] = shm_index1;
		shm_write_pos = (shm_write_pos+1)%1024;	
		memcpy(shared_index_memory_start+4, &shm_write_pos, 4);
		printf("shm_write_pos::%d\n", index_head->write_pos);

		shm_write_offset = 8 + 44;
		memcpy(shared_frame_memory_start+4, &shm_write_offset, 4);
		printf("shm_write_offset::%d\n", frame_head->write_offset);
		printf("%s\n", packet->frame);
		printf("********************IFrame No.1*******************\n");
		//*******************************IFrame No.2********************************
		packet = (FRAME_PACKET *)((char *)packet + 44);
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
  		strncpy(packet->frame, "ghijklabcdef", 12);

		P4VEM_ShMIndex_t shm_index2;
		shm_index2.type = 0x32;
		shm_index2.channel = 0x01;
		shm_index2.time.year = 0x10;
		shm_index2.time.month = 0x08;
		shm_index2.time.day = 0x18;
		shm_index2.time.hour = 0x0A;
		shm_index2.time.minute = 0x05;
		shm_index2.time.second = 0x09;
		shm_index2.offset = 0x08 + 44;
		shm_index2.lenth = 0x2C;

		index[1] = shm_index2;
		shm_write_pos = (shm_write_pos+1)%1024;	
		memcpy(shared_index_memory_start+4, &shm_write_pos, 4);
		printf("shm_write_pos::%d\n", index_head->write_pos);
	
		shm_write_offset = 8 + 88;
		memcpy(shared_frame_memory_start+4, &shm_write_offset, 4);
		printf("shm_write_offset::%d\n", frame_head->write_offset);
		printf("%s\n", packet->frame);
		printf("********************IFrame No.2*******************\n");
		//*******************************IFrame No.3********************************
		packet = (FRAME_PACKET *)((char *)packet + 44);
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
  		packet->rtc.stuRtcTime.cSecond = 0x0A;
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
		shm_index3.time.second = 0x0A;
		shm_index3.offset = 0x08 + 88;
		shm_index3.lenth = 0x2C;

		index[2] = shm_index3;
		shm_write_pos = (shm_write_pos+1)%1024;	
		memcpy(shared_index_memory_start+4, &shm_write_pos, 4);
		printf("shm_write_pos::%d\n", index_head->write_pos);

		shm_write_offset = 8 + 132;
		memcpy(shared_frame_memory_start+4, &shm_write_offset, 4);
		printf("shm_write_offset::%d\n", frame_head->write_offset);
		printf("%s\n", packet->frame);
		printf("********************IFrame No.3*******************\n");
		//*******************************PFrame No.4********************************
		packet = (FRAME_PACKET *)((char *)packet + 44);
		printf("%X\n", (int)packet);
		packet->head.IFrameType = 0x63643300;
  		packet->head.IFrameLen = 0x00000A;
  		packet->head.ISreamExam = 0xD6;
  		packet->head.IExtendLen = 0x000008;
  		packet->head.IExtendCount = 0x01;
		packet->video.stuInfoTYpe.LInfoType = 0x01;
		packet->video.stuInfoTYpe.LInfoLength = 0x000008;
		packet->video.IWidth = 0x280;
		packet->video.IHeight = 0x140;
		packet->video.IFPS = 0x1E;

  		strncpy((void*)packet + 20, "ghijklabcdef", 12);

		P4VEM_ShMIndex_t shm_index4;
		shm_index4.type = 0x33;
		shm_index4.channel = 0x01;
		shm_index4.time.year = 0x00;
		shm_index4.time.month = 0x00;
		shm_index4.time.day = 0x00;
		shm_index4.time.hour = 0x00;
		shm_index4.time.minute = 0x00;
		shm_index4.time.second = 0x00;
		shm_index4.offset = 0x08+132;
		shm_index4.lenth = 0x20;

		index[3] = shm_index4;
		shm_write_pos = (shm_write_pos+1)%1024;	
		memcpy(shared_index_memory_start+4, &shm_write_pos, 4);
		printf("shm_write_pos::%d\n", index_head->write_pos);

		shm_write_offset = 8 + 164;
		memcpy(shared_frame_memory_start+4, &shm_write_offset, 4);
		printf("shm_write_offset::%d\n", frame_head->write_offset);
		printf("%s\n", packet->frame);
		printf("********************IFrame No.4*******************\n");
		//*******************************IFrame No.5********************************
		packet = (FRAME_PACKET *)((char *)packet + 32);
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
  		packet->rtc.stuRtcTime.cSecond = 0x0A;
  		packet->rtc.stuRtcTime.usMilliSecond = 0;
  		packet->rtc.stuRtcTime.usWeek = 2;
  		packet->rtc.stuRtcTime.usReserved = 0;
  		packet->rtc.stuRtcTime.usMilliValidate = 0;
  		strncpy(packet->frame, "abcdefghijkl", 12);

		P4VEM_ShMIndex_t shm_index5;
		shm_index5.type = 0x32;
		shm_index5.channel = 0x01;
		shm_index5.time.year = 0x10;
		shm_index5.time.month = 0x08;
		shm_index5.time.day = 0x18;
		shm_index5.time.hour = 0x0A;
		shm_index5.time.minute = 0x05;
		shm_index5.time.second = 0x0A;
		shm_index5.offset = 0x08+164;
		shm_index5.lenth = 0x2C;

		index[4] = shm_index5;
		shm_write_pos = (shm_write_pos+1)%1024;	
		memcpy(shared_index_memory_start+4, &shm_write_pos, 4);
		printf("shm_write_pos::%d\n", index_head->write_pos);

		shm_write_offset = 8 + 208;
		memcpy(shared_frame_memory_start+4, &shm_write_offset, 4);
		printf("shm_write_offset::%d\n", frame_head->write_offset);
		printf("%s\n", packet->frame);
		printf("********************IFrame No.5*******************\n");
		//*******************************IFrame No.6********************************
		packet = (FRAME_PACKET *)((char *)packet + 44);
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
  		strncpy(packet->frame, "ghijklabcdef", 12);

		P4VEM_ShMIndex_t shm_index6;
		shm_index6.type = 0x32;
		shm_index6.channel = 0x01;
		shm_index6.time.year = 0x10;
		shm_index6.time.month = 0x08;
		shm_index6.time.day = 0x18;
		shm_index6.time.hour = 0x0A;
		shm_index6.time.minute = 0x05;
		shm_index6.time.second = 0x0C;
		shm_index6.offset = 0x08+208;
		shm_index6.lenth = 0x2C;

		index[5] = shm_index6;
		shm_write_pos = (shm_write_pos+1)%1024;	
		memcpy(shared_index_memory_start+4, &shm_write_pos, 4);
		printf("shm_write_pos::%d\n", index_head->write_pos);

		shm_write_offset = 8 + 252;
		memcpy(shared_frame_memory_start+4, &shm_write_offset, 4);
		printf("shm_write_offset::%d\n", frame_head->write_offset);
		printf("%s\n", packet->frame);
		printf("********************IFrame No.6*******************\n");

  }

  if (shmdt(shared_frame_memory_start) == -1) {
    fprintf(stderr, "shmdt failed\n");
    exit(EXIT_FAILURE);
  }

  if (shmdt(shared_index_memory_start) == -1) {
    fprintf(stderr, "shmdt failed\n");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}




