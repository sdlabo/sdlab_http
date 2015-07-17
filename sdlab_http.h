#ifndef SDLAB_HTTP_H
#define SDLAB_HTTP_H

#define SDLAB_ERROR(x) {                        \
    fprintf(stderr,"error ");                   \
    perror(x);                                  \
  }
#define PRINT_IP(a) {                                   \
  printf("%d.%d.%d.%d\n",(0x0ff&(a>>24)),               \
         (0x0ff&(a>>16)),(0x0ff&(a>>8)),(0x0ff&a));     \

#define DATA_BURST_SIZE 512
#define SERVER_PORT 8000
#define SDLAB_UDP_BUF_LEN 10240


typedef struct
{
  int sock;
}thread_type;


extern void sdlab_http_thread();

#endif

