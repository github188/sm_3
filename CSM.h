#ifndef __CSM_H_
#define __CSM_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<malloc.h>
#include<sys/stat.h>
#include<strings.h>
#include<errno.h>
#include<ctype.h>
#include<stdint.h>
#include<sys/queue.h>
#include<sys/un.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<netinet/in.h>
#include<unistd.h>
#include<fcntl.h>
#include<time.h>
#include<signal.h>
#include<pthread.h>

#define P4_MAGIC      20160704
#define TABLE_LEN     16
#define SERVER_LEN    16
#define SERVER_MASK   ( SERVER_LEN - 1 )
#define SERVER_BEGIN   0
typedef enum Bool
{
    FALSE = 0,
    TRUE  = 1,
}BOOL;


/* 
 * Decode-Encode can provide server type;
 * */
typedef enum DE_Server
{
    DE_FIRST = SERVER_BEGIN,   // 0
    DE_ENCODE,
    DE_LAST = SERVER_BEGIN + SERVER_MASK   // 0 + 15  = 15
}DE_Server;

/*
 * Storage Module can provide server types;
 * */
typedef enum Storage_Server
{
    SS_FIRST = DE_LAST + 1, // 15 + 1 = 16
    SS_STORAGE,
    SS_LAST = SS_FIRST + SERVER_MASK  // 16 + 15 = 31
}Storage_Server;

/*
 * Network Module can provide server types;
 * */
typedef enum Network_Server
{
    NF_FIRST = SS_LAST + 1,  // 31 + 1 = 32
    NF_TRANSPORT,
    NF_LAST  = NF_FIRST + SERVER_MASK  // 32 + 15 = 47
}Network_Server;

/*
 * Device Module can provide server types;
 * */
typedef enum Device_Server
{
    DS_FIRST = NF_LAST + 1,    // 47 + 1 = 48
    DS_GPS,
    DS_LED,
    DS_LAST  = DS_FIRST + SERVER_MASK  // 48 + 15 = 63
}Device_Server;

/*
 * 为每个模块服务起始编号;
 * */
typedef enum Module_Num
{
    DES_MODULE = DE_FIRST,
    SRM_MODULE = SS_FIRST,
    NET_MODULE = NF_FIRST,
    DEV_MODULE = DS_FIRST
}Module_Num;


typedef enum Type 
{
    REGISTER_TYPE     = 0,
    REQUEST_TYPE         ,
    TRANSFER_REQUEST_TYPE,
    REQUEST_RESPONSE_TYPE, 

    HEART_TYPE           ,
    SHARED_TYPE          ,
    TRANSFER_SHARED_TYPE ,
    SHARED_RESPONSE_TYPE ,

    ERRNO_TYPE           ,
}TYPE;

#pragma pack ( 1 )

/* struct base_proto 中存放的是各种类型的通信协议的公共部分;
 * */
struct base_proto 
{
    int32_t magic;
    int32_t type;
    pid_t    pid;
};

struct heart_proto
{
    struct base_proto base;
    int32_t    server;
};

struct register_proto
{
    struct base_proto  base;
    int32_t server;
    int32_t number; 
    char     *path;
};

struct request_proto
{
    struct base_proto base;
    int32_t server;
};

struct response_proto
{
    struct base_proto base;
    int32_t errnum; 
    int32_t errno_server;
};

struct sharem_proto
{
    struct base_proto base;
    int32_t     server;
    key_t       key;
};

struct P4
{
    struct register_proto p4_protocol;
};

struct register_table
{
    struct sockaddr_un domain;
    int32_t           server;
    int32_t           number;
    pid_t             pid;
    time_t             timeout;
    int8_t             count;
    char               *path;
};

#pragma pack( 0 )

static inline struct base_proto *get_base( void *p4 )
{
    return ( struct base_proto * )p4;
}

static inline struct heart_proto *get_heart_pro( struct P4 *p4 )
{
    return ( struct heart_proto * )p4;
}

static inline struct register_proto *get_reg_pro( struct P4 *p4 )
{
    return ( struct register_proto * )p4;
}

static inline struct request_proto *get_req_pro( struct P4 *p4 )
{
    return ( struct request_proto *)p4;
}

static inline struct response_proto *get_res_pro( struct P4 *p4 )
{
    return ( struct response_proto * )p4;
}

static inline struct sharem_proto *get_shm_pro( struct P4 *p4 )
{
    return ( struct sharem_proto * )p4;
}

static inline struct sockaddr_un *get_socket( struct register_table *reg )
{
    return ( struct sockaddr_un *)reg;
}

#endif
