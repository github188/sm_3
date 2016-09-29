#ifndef __COMMON_H_
#define __COMMON_H_

/*
 * create_socket ：用于创建一个域套接字;
 *
 * domain : 是域套接字所要绑定的路径; P4系统中的每一个模块所绑定的域套接字最后都存放在"/tmp/P4/socket/"目录下，所以建议 domain 格式为:"/tmp/P4/socket/模块名称.socket".
 * 
 * 返回值 : 返回域套接字描述符;
 * */
int create_socket( const char *domain);


/*
 * generate_message : 用于生成一个struct P4 结构体;
 *
 * type      : 为struct P4结构体所有负载的数据类型;
 *             其取值有:
 *                 REGISTER_TYPE  为注册类;
 *                 REQUEST_TYPE   为请求所需服务类;
 *                 RESPONSE_TYPE  为对请求进行的响应类;
 *                 HEART_TYPE     为心跳类;
 *
 * serverfd  : 为系统给每个模块的服务起始编号;
 *             其取值有:
 *                     DES_MODULE 为视频编码模块的服务起始码
 *                     SRM_MODULE 为存储模块的服务起始码
 *                     NET_MODULE 为网络模块的服务起始码
 *                     DEV_MODULE 为设备管理模块的服务起始码
 *
 * servernum : 为每个模块所能提供的服务数量;
 *             
 *
 * path      : 为每个模块的绝对路径; 由于P4系统中的所有模块最后都会存放在"/tmp/P4/module/"目录下，所有建议 path 的绝对路径格式为:"/tmp/P4/module/每个模块的名称";
 *
 * length    : 用于存放generate_message生成的数据的大小;
 * */
struct P4 * generate_message( uint32_t type, uint32_t serverfd, uint32_t servernum, const char *path, uint32_t *length );


/*
 * send_message : 用于发送generate_message所生成的struct P4结构体;
 *
 * sockfd       : 为发送方的域套接字描述符;
 * p4           : 为将要被发送的struct P4 结构体;
 * domain       : 为接收方的域套接字路径;
 * length       : 为将要发送的数据的大小;
 * */
int send_message( int sockfd, const struct P4 *p4, const char *domain, int length );


/*
 * recv_data : 用于接收数据;
 *
 * sockfd    : 为接收方的域套接字描述符;
 * buffer    : 用于存放接收数据的缓存;
 * buffer_size : 为缓存区的大小;
 * sock      : 用于存发送方的域套接字;
 * */
int recv_data( uint32_t sockfd, char *buffer, uint32_t buffer_size, struct sockaddr_un *sock );


#if 0
/*
 * request_process : 生成一个struct P4 结构体，用该结构体来存放中心调度模块转发过来的请求数据;
 * */
struct P4 *request_process( char *buffer );
#endif 

#endif
