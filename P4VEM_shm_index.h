#ifndef _P4VEM_SHM_INDEX_H
#define _P4VEM_SHM_INDEX_H

#include <stdint.h>

//帧类型(在P4VEM_FrameHeaderDef.h中已定义)
//#define VEM_FRAME_TYPE_I		50	//I帧：x2dc，ASCII('2') <==> HEX(32) <==> DEC(50)
//#define VEM_FRAME_TYPE_P		51	//P帧：x3dc，ASCII('3') <==> HEX(33) <==> DEC(51)
//#define VEM_FRAME_TYPE_AUDIO	52	//音频帧：x4dc，ASCII('4') <==> HEX(34) <==> DEC(52)
//#define VEM_FRAME_TYPE_265_I	53	//h265-I帧：x5dc，ASCII('5') <==> HEX(35) <==> DEC(53)
//#define VEM_FRAME_TYPE_265_P	54	//h265-P帧：x6dc，ASCII('6') <==> HEX(36) <==> DEC(54)
//#define VEM_FRAME_TYPE_MJPEG	55	//MJPEG帧：x7dc，ASCII('7') <==> HEX(37) <==> DEC(55)

/*
* 共享内存common
* 帧索引共享内存结构
*  HEAD+INDEX_DATA
*  HEAD:read_pos(4 bytes) + write_pos(4 bytes)
*
* 帧数据共享内存结构 HEAD+FRAME_DATA
*  HEAD+INDEX_DATA
*  HEAD:read_offset(4 bytes) + write_offset(4 bytes)
*/
//共享内存类型
#define SHM_TYPE_INDEX  1   //帧索引
#define SHM_TYPE_DATA   2   //帧数据

//共享内存头部大小（单位：字节）
#define SHM_IND_HEAD_SIZE   8   //索引shm
#define SHM_DAT_HEAD_SIZE   8   //数据shm

//共享内存总大小（单位：字节）
#define SHM_IND_TOTAL_SIZE  (1024*16 + SHM_IND_HEAD_SIZE)   //索引shm头部大小
#define SHM_DAT_TOTAL_SIZE  1024*1024*4                     //数据shm头部大小



//时间
typedef struct P4VEM_IndexTime
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}P4VEM_IndexTime_t;

//帧索引
typedef struct P4VEM_ShMIndex
{
	uint8_t	type;	//帧类型
	uint8_t	channel;//通道号
	P4VEM_IndexTime_t	time;	//时间戳
	uint32_t	offset;	//帧偏移量
	uint32_t	lenth;	//帧长度
}P4VEM_ShMIndex_t;

//帧数据共享内存头部
typedef struct P4VEM_ShM_DAT_HEAD
{
	uint32_t read_offset;   //已写帧数据下一位置偏移量
	uint32_t write_offset;  //已读帧数据下一位置偏移量
}P4VEM_ShM_DAT_HEAD_t;

//帧索引共享内存头部
typedef struct P4VEM_ShM_IND_HEAD
{
    uint32_t read_pos;      //读下标（下一位置）
    uint32_t write_pos;     //写下标（下一位置）
}P4VEM_ShM_IND_HEAD_t;

#endif // !_P4VEM_SHM_INDEX_H

