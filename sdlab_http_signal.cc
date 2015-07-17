#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <boost/thread.hpp>
#include "sdlab.h"
#include "sdlab_http.h"
#include "sdlab_http_signal.h"

int phase_offset = 0;

boost::mutex mutex_sine;

boost::mutex sdlab_mutex_channel_a;
boost::mutex sdlab_mutex_channel_b;

boost::mutex sdlab_mutex_cross1;
boost::mutex sdlab_mutex_cross10;
boost::mutex sdlab_mutex_cross_total;

double sdlab_channel_a_buf[SDLAB_PLOT_LEN];
double sdlab_channel_b_buf[SDLAB_PLOT_LEN];

double sdlab_cross1_re_buf[SDLAB_PLOT_LEN];
double sdlab_cross1_im_buf[SDLAB_PLOT_LEN];
double sdlab_fft1_a_buf[SDLAB_PLOT_LEN];
double sdlab_fft1_b_buf[SDLAB_PLOT_LEN];

double sdlab_cross10_re_buf[SDLAB_PLOT_LEN];
double sdlab_cross10_im_buf[SDLAB_PLOT_LEN];
double sdlab_fft10_a_buf[SDLAB_PLOT_LEN];
double sdlab_fft10_b_buf[SDLAB_PLOT_LEN];

double sdlab_cross_total_re_buf[SDLAB_PLOT_LEN];
double sdlab_cross_total_im_buf[SDLAB_PLOT_LEN];
double sdlab_fft_total_a_buf[SDLAB_PLOT_LEN];
double sdlab_fft_total_b_buf[SDLAB_PLOT_LEN];

int noprintf(const char *s, ...)
{
  return 0;
}

int sprintf_sinewave(char* buf, double amp, double phase, int num)
{
  int i;
  int len = 0;

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * sin(phase));
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * sin(((double) i)/100.0
                                                * 2.0 * M_PI + phase));
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}


int sprintf_channel_a(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_udpwave");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_channel_a);
    for(i = 0; i < num; i++){
      tmpbuf[i] = ((double)sdlab_channel_a_buf[i]) / 65535.0 * 100.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}


int sprintf_channel_b(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_channel_b");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_channel_b);
    for(i = 0; i < num; i++){
      tmpbuf[i] = ((double)sdlab_channel_b_buf[i]) / 65535.0 * 100.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}

int sprintf_cross1_re(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_cross1_re");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_cross1);
    for(i = 0; i < num; i++){
      tmpbuf[i] = 0.000000000001 * sdlab_cross1_re_buf[i] + 50.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}

int sprintf_cross1_im(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_cross1_im");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_cross1);
    for(i = 0; i < num; i++){
      tmpbuf[i] = 0.000000000001 * sdlab_cross1_im_buf[i] + 50.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}

int sprintf_fft1_a(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_fft1_a");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_cross1);
    for(i = 0; i < num; i++){
      tmpbuf[i] = 0.000001 * sdlab_fft1_a_buf[i] + 25.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");

  return len;
}

int sprintf_fft1_b(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_fft1_b");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_cross1);
    for(i = 0; i < num; i++){
      tmpbuf[i] = 0.000001 * sdlab_fft1_b_buf[i] + 25.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}

//10sec

int sprintf_cross10_re(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_cross10_re");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_cross1);
    for(i = 0; i < num; i++){
      tmpbuf[i] = 0.000000000001 * sdlab_cross10_re_buf[i] + 50.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}

int sprintf_cross10_im(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_cross10_im");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_cross1);
    for(i = 0; i < num; i++){
      tmpbuf[i] = 0.000000000001 * sdlab_cross10_im_buf[i] + 50.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}

int sprintf_fft10_a(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_fft10_a");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_cross1);
    for(i = 0; i < num; i++){
      tmpbuf[i] = 0.000001 * sdlab_fft10_a_buf[i] + 25.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}

int sprintf_fft10_b(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_fft10_b");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_cross1);
    for(i = 0; i < num; i++){
      tmpbuf[i] = 0.000001 * sdlab_fft10_b_buf[i] + 25.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}


//10sec

int sprintf_cross_total_re(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_cross_total_re");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_cross1);
    for(i = 0; i < num; i++){
      tmpbuf[i] = 0.000000000001 * sdlab_cross_total_re_buf[i] + 50.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}

int sprintf_cross_total_im(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_cross_total_im");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_cross1);
    for(i = 0; i < num; i++){
      tmpbuf[i] = 0.000000000001 * sdlab_cross_total_im_buf[i] + 50.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}

int sprintf_fft_total_a(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_fft_total_a");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_cross1);
    for(i = 0; i < num; i++){
      tmpbuf[i] = 0.000001 * sdlab_fft_total_a_buf[i] + 25.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}

int sprintf_fft_total_b(char* buf, double amp, int num)
{
  int i;
  int len = 0;
  double tmpbuf[SDLAB_PLOT_LEN];

  if(num > SDLAB_PLOT_LEN){
    fprintf(stderr,"error ");
    perror("sprintf_fft_total_b");
    exit(0);
  }

  {
//    boost::mutex::scoped_lock lock(sdlab_mutex_cross1);
    for(i = 0; i < num; i++){
      tmpbuf[i] = 0.000001 * sdlab_fft_total_b_buf[i] + 25.0;
    }
  }

  len += sprintf(buf, "[");
  len += sprintf(buf + len, "%f", amp * tmpbuf[0]);
  for(i = 1; i < num; i++){
    len += sprintf(buf + len, ", %f", amp * tmpbuf[i]);
  }
  len += sprintf(buf + len, "]");
  if(len > SDLAB_PLOT_LEN){
    printf("too small SDLAB_PLOT_LEN %s %s %d\n",
           __FILE__, __FUNCTION__, __LINE__);
    exit(1);
  }

  return len;
}

//send functions starts



void send_sinewave(int sock)
{
  char buf[12048];
  char sinbuf[12048];
  int len = 0;
  int slen = 0;
  int ret;
  double phase;

  {
    boost::mutex::scoped_lock lock(mutex_sine);
    phase_offset++;
    phase = ((double) phase_offset) * 2 * M_PI / 30.0;
  }

  slen = sprintf_sinewave(sinbuf, 10.0, phase, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", slen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("sinewave: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_sinewave");
  }

//  printf("%s", sinbuf);
  DEBUG("sinewave: sending sinebuf\n");
  ret = send(sock, sinbuf, slen, 0);
  DEBUG("sinewave: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("sine wave failed");
  }
  DEBUG("sinewave: finished\n");
}

void send_channel_a(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_channel_a(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("udpwave: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_udpwave0");
  }

//  printf("%s", sinbuf);
  DEBUG("udpwave0: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("udpwave0: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("udpwave: finished\n");
}

void send_channel_b(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_channel_b(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("udpwave: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_udpwave0");
  }

//  printf("%s", sinbuf);
  DEBUG("udpwave0: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("udpwave0: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("udpwave: finished\n");
}

//1sec

void send_cross1_re(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_cross1_re(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("udpwave: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_cross1_re");
  }

//  printf("%s", sinbuf);
  DEBUG("cross1_re: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("cross1_re: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("udpwave: finished\n");
}

void send_cross1_im(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_cross1_im(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("udpwave: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_cross1_im");
  }

//  printf("%s", sinbuf);
  DEBUG("cross1_im: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("cross1_im: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("udpwave: finished\n");
}

void send_fft1_a(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_fft1_a(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("udpwave: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_fft1_a");
  }

//  printf("%s", sinbuf);
  DEBUG("fft1_a: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("fft1_a: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("udpwave: finished\n");
}

void send_fft1_b(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_fft1_b(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("udpwave: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_fft1_b");
  }

//  printf("%s", sinbuf);
  DEBUG("fft1_b: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("fft1_b: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("udpwave: finished\n");
}

//10sec

void send_cross10_re(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_cross10_re(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("cross10_re: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_cross10_re");
  }

//  printf("%s", sinbuf);
  DEBUG("cross10_re: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("cross10_re: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("cross10_re: finished\n");
}

void send_cross10_im(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_cross10_im(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("cross10_im: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_cross10_im");
  }

//  printf("%s", sinbuf);
  DEBUG("cross10_im: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("cross10_im: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("cross10_im: finished\n");
}

void send_fft10_a(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_fft10_a(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("udpwave: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_fft10_a");
  }

//  printf("%s", sinbuf);
  DEBUG("fft10_a: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("fft10_a: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("udpwave: finished\n");
}

void send_fft10_b(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_fft10_b(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("udpwave: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_fft10_b");
  }

//  printf("%s", sinbuf);
  DEBUG("fft10_b: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("fft10_b: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("udpwave: finished\n");
}

//total

void send_cross_total_re(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_cross_total_re(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("cross_total_re: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_cross_total_re");
  }

//  printf("%s", sinbuf);
  DEBUG("cross_total_re: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("cross_total_re: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("cross_total_re: finished\n");
}

void send_cross_total_im(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_cross_total_im(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("cross_total_im: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_cross_total_im");
  }

//  printf("%s", sinbuf);
  DEBUG("cross_total_im: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("cross_total_im: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("cross_total_im: finished\n");
}

void send_fft_total_a(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_fft_total_a(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("udpwave: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_fft_total_a");
  }

//  printf("%s", sinbuf);
  DEBUG("fft_total_a: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("fft_total_a: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("udpwave: finished\n");
}

void send_fft_total_b(int sock)
{
  char buf[12048];
  char udpbuf[12048];
  int len = 0;
  int ulen = 0;
  int ret;

  ulen = sprintf_fft_total_b(udpbuf, 1.0, 1000);

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", ulen);
  len += sprintf(buf + len,
                 "Content-Type: application/json; charset=utf-8\r\n");
  len += sprintf(buf + len, "\r\n");

//  printf("%s", buf);
  DEBUG("udpwave: sending header\n");
  ret = send(sock, buf, len, 0);
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_fft_total_b");
  }

//  printf("%s", sinbuf);
  DEBUG("fft_total_b: sending udpbuf\n");
  ret = send(sock, udpbuf, ulen, 0);
  DEBUG("fft_total_b: sending sinebuf finished\n");
  if(ret < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("udp wave failed");
  }
  DEBUG("udpwave: finished\n");
}

