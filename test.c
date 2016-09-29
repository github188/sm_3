#include <stdio.h>

/*
编译器中提供了#pragma pack(n)来设定变量以n字节对齐方式。n字节对齐就是说变量存放的起始地址的偏移量有两种情况：
第一、如果n大于等于该变量所占用的字节数，那么偏移量必须满足变量的默认的对齐方式;
第二、如果n小于该变量的类型所占用的字节数，那么偏移量为n的倍数，不用满足变量默认的对齐方式。

结构的总大小也有个约束条件，分下面两种情况：
如果n大于所有成员变量类型所占用的字节数，那么结构的总大小必须为占用空间最大的变量占用的空间数的倍数；否则必须为n的倍数
*/

#pragma pack(1)

struct node
{
  char f;
  int e;
  short int a;
  char b;
};

typedef enum Type 
{
    REGISTER_TYPE = 0,
    REQUEST_TYPE     ,
    RESPONSE_TYPE    ,
    HEART_TYPE       ,
    SHARED_TYPE      ,
    ERRNO_TYPE       ,
}TYPE;


int main(void)
{
	//struct node n;
	//printf("%d\n",sizeof(n));

	TYPE a;
	a = REQUEST_TYPE;
	printf("%d\n", a);
	
	return 0;
}






