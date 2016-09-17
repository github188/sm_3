#ifndef _P4VEM_SHM_INDEX_H
#define _P4VEM_SHM_INDEX_H

#include <stdint.h>

//֡����(��P4VEM_FrameHeaderDef.h���Ѷ���)
//#define VEM_FRAME_TYPE_I		50	//I֡��x2dc��ASCII('2') <==> HEX(32) <==> DEC(50)
//#define VEM_FRAME_TYPE_P		51	//P֡��x3dc��ASCII('3') <==> HEX(33) <==> DEC(51)
//#define VEM_FRAME_TYPE_AUDIO	52	//��Ƶ֡��x4dc��ASCII('4') <==> HEX(34) <==> DEC(52)
//#define VEM_FRAME_TYPE_265_I	53	//h265-I֡��x5dc��ASCII('5') <==> HEX(35) <==> DEC(53)
//#define VEM_FRAME_TYPE_265_P	54	//h265-P֡��x6dc��ASCII('6') <==> HEX(36) <==> DEC(54)
//#define VEM_FRAME_TYPE_MJPEG	55	//MJPEG֡��x7dc��ASCII('7') <==> HEX(37) <==> DEC(55)

/*
* �����ڴ�common
* ֡���������ڴ�ṹ
*  HEAD+INDEX_DATA
*  HEAD:read_pos(4 bytes) + write_pos(4 bytes)
*
* ֡���ݹ����ڴ�ṹ HEAD+FRAME_DATA
*  HEAD+INDEX_DATA
*  HEAD:read_offset(4 bytes) + write_offset(4 bytes)
*/
//�����ڴ�����
#define SHM_TYPE_INDEX  1   //֡����
#define SHM_TYPE_DATA   2   //֡����

//�����ڴ�ͷ����С����λ���ֽڣ�
#define SHM_IND_HEAD_SIZE   8   //����shm
#define SHM_DAT_HEAD_SIZE   8   //����shm

//�����ڴ��ܴ�С����λ���ֽڣ�
#define SHM_IND_TOTAL_SIZE  (1024*16 + SHM_IND_HEAD_SIZE)   //����shmͷ����С
#define SHM_DAT_TOTAL_SIZE  1024*1024*4                     //����shmͷ����С



//ʱ��
typedef struct P4VEM_IndexTime
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}P4VEM_IndexTime_t;

//֡����
typedef struct P4VEM_ShMIndex
{
	uint8_t	type;	//֡����
	uint8_t	channel;//ͨ����
	P4VEM_IndexTime_t	time;	//ʱ���
	uint32_t	offset;	//֡ƫ����
	uint32_t	lenth;	//֡����
}P4VEM_ShMIndex_t;

//֡���ݹ����ڴ�ͷ��
typedef struct P4VEM_ShM_DAT_HEAD
{
	uint32_t read_offset;   //��д֡������һλ��ƫ����
	uint32_t write_offset;  //�Ѷ�֡������һλ��ƫ����
}P4VEM_ShM_DAT_HEAD_t;

//֡���������ڴ�ͷ��
typedef struct P4VEM_ShM_IND_HEAD
{
    uint32_t read_pos;      //���±꣨��һλ�ã�
    uint32_t write_pos;     //д�±꣨��һλ�ã�
}P4VEM_ShM_IND_HEAD_t;

#endif // !_P4VEM_SHM_INDEX_H

