#include "CSM.h"

#define BUFFER_SIZE  256

int create_socket(const char *domain)
{
    int sockfd;
    struct sockaddr_un client;
    bzero( &client, sizeof( struct sockaddr_un ) );

    client.sun_family = AF_UNIX;
    strncpy( client.sun_path, domain, strlen( domain) );
    unlink( domain );

    sockfd = socket( AF_UNIX, SOCK_DGRAM, IPPROTO_IP );
    if( sockfd  < 0 )
    {
        fprintf( stderr, "%s:%d socket fail reason is : %s!\n", __FILE__, __LINE__, strerror( errno ) );
        exit( -1 );
    }

    if( bind( sockfd , ( struct sockaddr *)&client, sizeof( struct sockaddr_un ) ) < 0 )
    {
        fprintf( stderr, "%s:%d bind fail reason is : %s!\n", __FILE__, __LINE__, strerror( errno  ) );
        exit( -1 );
    }

    return sockfd;
}

struct P4 *generate_message( uint32_t  type , uint32_t serverfd, uint32_t servernum, const char *path, uint32_t *size )
{
    int length ;
    struct P4 *p4 = NULL;
    if( path != NULL )
    { 
        length = sizeof( struct P4 ) + strlen( path ) + 1;
    }
    else
        length = sizeof( struct P4 );
    p4 = ( struct P4 * )malloc( length );
    if( p4 == NULL )
    {
        fprintf( stderr, "%s : %d fail reason is : %s\n", __FILE__, __LINE__, strerror( errno ) );
    }
    memset( p4, 0, length );

    struct base_proto     *base ;
    struct register_proto *reg;
    struct request_proto  *req;
    struct heart_proto    *heart;
    struct sharem_proto   *sharem;
    base = get_base(p4);
    base->magic = P4_MAGIC;
    base->type = type;
    base->pid = getpid( );

    switch( type )
    {
        case REGISTER_TYPE:
            reg = get_reg_pro( p4 );
            reg->server = serverfd;
            reg->number = servernum;
            reg->path = ( char *)reg + sizeof( struct register_proto );
            strncpy( reg->path, path, strlen( path ) );
            *size = length;
            break;
        case REQUEST_TYPE:
            req = get_req_pro( p4 );
            req->server = serverfd;
            *size = sizeof( struct request_proto );
            break;
        case HEART_TYPE:
            //            printf( "heart type!\n" );
            heart = get_heart_pro( p4 );
            heart->server = serverfd;
            *size = sizeof( struct heart_proto );
            break;
        case SHARED_TYPE:
            sharem = get_shm_pro( p4 );
            sharem->server = serverfd;
            *size = sizeof( struct sharem_proto );
            break;
    }
    return p4;
}

int send_message( uint32_t sockfd, const struct P4 *p4, const uint8_t *domain, uint32_t  length )
{
    int ret, socket_len;
    struct sockaddr_un server;
    memset( &server, 0, sizeof( struct sockaddr_un ) );
    server.sun_family = AF_UNIX;
    strncpy( server.sun_path, domain, strlen( domain ) );

    socket_len = offsetof( struct sockaddr_un, sun_path ) + strlen( server.sun_path );

    while( ( ( ret = sendto( sockfd, p4, length, 0 , ( struct sockaddr *)&server, socket_len ) ) == -1 ) && ( errno == EINTR ) );
    if( ret < 0 )
    {
        fprintf( stderr, "%s : %d sendto fail reason is : %s\n", __FILE__, __LINE__, strerror( errno ) );
    }
    return ret;
}

int recv_data(uint32_t sockfd, char *buffer, uint32_t buffer_size, struct sockaddr_un *sock)
{
    uint32_t ret = 0;
    uint32_t len = 0;
    memset( buffer, 0, buffer_size );
    len = sizeof( struct sockaddr_un );

    ret = recvfrom( sockfd, buffer, buffer_size, 0, ( struct sockaddr *)sock, ( socklen_t *)&len );

    return ret;   
}

#if 0
struct P4 * request_process(char *buffer)
{ 
    uint32_t length;
    char      *domain;

    struct P4 *p4, *p4_tmp;
    struct base_proto *base, *base_tmp;
    struct register_proto *reg, *reg_tmp;

    p4 = ( struct P4 *)buffer;
    base = get_base(p4);
    reg  = get_reg_pro( p4 );
    
    domain = ( char *)reg + sizeof( struct register_proto );

    length = sizeof( struct register_proto ) + strlen( domain ) + 1;

    p4_tmp = ( struct P4 *)malloc( length );
    if( p4_tmp == NULL )
        fprintf( stderr, "%s:%d reason fail is : %s\n", __FILE__, __LINE__, strerror( errno ) );

    memset( p4_tmp, 0, length );
    base_tmp = get_base( p4_tmp );
    base_tmp->magic = base->magic;
    base_tmp->type  = RESPONSE_TYPE;
    base_tmp->pid   = base->pid;
    
    reg_tmp = get_reg_pro( p4_tmp );
    strncpy( reg_tmp->path, domain, strlen( domain ) );

    return p4_tmp;
}

#endif 
