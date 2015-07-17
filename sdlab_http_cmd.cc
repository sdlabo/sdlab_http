#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>/* struct hostent,gethostbyname‚Ì‚½‚ß */
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <boost/thread.hpp>
#include "sdlab.h"
#include "sdlab_http.h"
#include "sdlab_udpfft.h"
#include "sdlab_http_cmd.h"

void cmd_reset(int sock, info_type *info)
{
  char buf[1024];
  char content[1024];

  sdlab_udpfft_reset();
  int ulen = sprintf(content, "<html><head><meta http-equiv=\"refresh\" content=\"1 ; URL=%s\">", info->referer);

  DEBUG("cmd_resetting\n");
  
  int len = sprintf(buf, "HTTP/1.1 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len, "Connection: close\r\n");
  len += sprintf(buf + len,
                 "Content-Type: text/html; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

  DEBUG("%s", buf);
  int ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_fft1_a");
  }
  DEBUG("ret send %d\n", ret);

  DEBUG("%s", content);
  ret = send(sock, content, ulen, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("ret send %d\n", ret);  
}


void cmd_start(int sock, info_type *info)
{
  char buf[1024];
  char content[1024];

  sdlab_start_save();
  int ulen = sprintf(content, "<html><head><meta http-equiv=\"refresh\" content=\"1 ; URL=%s\">", info->referer);

  printf("cmd_resetting\n");
  
  int len = sprintf(buf, "HTTP/1.1 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len, "Connection: close\r\n");
  len += sprintf(buf + len,
                 "Content-Type: text/html; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

  DEBUG("%s", buf);
  int ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_fft1_a");
  }
  DEBUG("ret send %d\n", ret);

  DEBUG("%s", content);
  ret = send(sock, content, ulen, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("ret send %d\n", ret);
}

void cmd_stop(int sock, info_type *info)
{
  char buf[1024];
  char content[1024];

  sdlab_stop_save();
  int ulen = sprintf(content, "<html><head><meta http-equiv=\"refresh\" content=\"1 ; URL=%s\">", info->referer);

  DEBUG("cmd_stop\n");
  
  int len = sprintf(buf, "HTTP/1.1 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len, "Connection: close\r\n");
  len += sprintf(buf + len,
                 "Content-Type: text/html; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

  DEBUG("%s", buf);
  int ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_fft1_a");
  }
  DEBUG("ret send %d\n", ret);

  DEBUG("%s", content);
  ret = send(sock, content, ulen, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("ret send %d\n", ret);
}
