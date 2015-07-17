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
#include "sdlab.h"

void get_status(int sock, info_type *info)
{
  char buf[1024];
  char content[1024];

  int ulen = sprintf(content, "{\"calc_max\": %f,", log_calc_max);
  ulen += sprintf(content + ulen, "\"calc_min\": %f,", log_calc_min);
  ulen += sprintf(content + ulen, "\"calc_now\": %f,", log_calc_now);
  ulen += sprintf(content + ulen, "\"fft_max\": %f,", log_fft_max);
  ulen += sprintf(content + ulen, "\"fft_min\": %f,", log_fft_min);
  ulen += sprintf(content + ulen, "\"fft_now\": %f,", log_fft_now);
  ulen += sprintf(content + ulen, "\"drop_count\": %d}", log_drop_count);

  DEBUG("cmd_resetting\n");
  
  int len = sprintf(buf, "HTTP/1.1 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len, "Connection: close\r\n");
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
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
