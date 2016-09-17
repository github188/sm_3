#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

struct test
{
	unsigned char a[20];
};

void fun(void)
{
	char *p = (char *)malloc(69);
	printf("%d-%d-%d-%d\n", p[0], p[1], p[2], p[3]);
	p[0] = 1;
	p[1] = 2;
	p[2] = 3;
	p[3] = 4;
	printf("%#x\n", (int)p);
	free(p);
}

int main1()
{
	/*int fd = open("tmp", O_RDWR | O_TRUNC);
	printf("%d\n", fd);
	while(1)
	{
		write(fd, "123456", 6);
		usleep(10000);
	}
	int pos1 = lseek(fd, 0, SEEK_END);
	int pos2 = lseek(fd, 0, SEEK_CUR);
	printf("%d-%d\n", pos1, pos2);

	fd = open("tmp", O_RDWR | O_APPEND);
	printf("%d\n", fd);
	write(fd, "654321", 6);
	int pos3 = lseek(fd, 0, SEEK_CUR);
	printf("%d\n", pos3);*/

	/*int a[5] = {0};
	int *p1 = a;
	int *p2 = &a;
	printf("%x-%x-%x\n", a, a+1, &a+1);*/

	/*struct test b;
	memset(&b, 0, 20);
	memcpy(b.a, "abcd", 4);
	printf("%c\n", b.a[0]);*/

	//fun();
	//fun();

	return 0;
}

#include <stdlib.h>  
#include <stdio.h>  
#include <unistd.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
  
int main() 
{
     /*fflush(stdout);  
     setvbuf(stdout,NULL,_IONBF,0);  
     printf("test stdout\n");  
     int save_fd = dup(STDOUT_FILENO); // 保存标准输出 文件描述符 注:这里一定要用 dup 复制一个文件描述符. 不要用 = 就像是Winodws下的句柄.  
     int fd = open("tmp",(O_RDWR | O_CREAT), 0644);  
     dup2(fd,STDOUT_FILENO); // 用我们新打开的文件描述符替换掉 标准输出  
     printf("test file\n");  
       
     //再恢复回来标准输出. 两种方式  
     //方法1 有保存 标准输出的情况  
     dup2(save_fd,STDOUT_FILENO);  
       
     //方法2 没有保存 标准输出的情况  
     //int ttyfd = open("/dev/tty",(O_RDWR), 0644);  
     //dup2(ttyfd,STDOUT_FILENO);  
     printf("test tty\n");  */

	close(1);
	int fd = open("tmp", O_RDWR | O_CREAT, 0644);
	printf("fd:%d\n", fd);
	return 0;
}






