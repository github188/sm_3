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

typedef struct P4VEM_ShMHEAD
{
	uint32_t read_offset; 
	uint32_t write_offset;
}P4VEM_ShMHEAD_t;

#endif // !_P4VEM_SHM_INDEX_H

