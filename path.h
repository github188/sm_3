#ifndef __PATH_H_
#define __PATH_H_

/*
 * paths中存放的是每个模块所在的绝对路径;
 * */
char *paths[] = { 
    "/tmp/P4/module/des",  // 大表哥的视频编码模块路径;
    "/tmp/P4/module/srm",  // 坤哥存储模块路径;
    "/tmp/P4/module/net",  // 教主的网络模块路径;
    "/tmp/P4/module/dev",
    "/tmp/P4/module/log",
};

char *sockets[] = { 
    "/tmp/P4/socket/des.socket", //大表哥视频编码模块所用域套接字文件名;
    "/tmp/P4/socket/srm.socket", //坤哥存储模块所用域套接字文件名;
    "/tmp/P4/socket/net.socket", //教主网络模块所用域套接字文件名;
    "/tmp/P4/socket/dev.socket",
    "/tmp/P4/socket/log.socket",
};

/*
 * server_file 为中心调度模块所用域套接字文件名;
 * */
const char *const server_file = "/tmp/P4/socket/recv.socket";  

#endif
