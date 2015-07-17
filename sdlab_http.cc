#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>/* struct hostent,gethostbynameのため */
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "sdlab.h"
#include "sdlab_http.h"
#include "sdlab_http_signal.h"
#include "sdlab_http_cmd.h"
#include "sdlab_http_status.h"

enum header_end_type{
  SEARCH_END,
  CR,
  CRLF,
  CRLFCR,
  CRLFCRLF
};

enum parse_state_type{
  PARSE_STATUS,
  PARSE_NAME,
  PARSE_VALUE,
  SKIP_LF
};


int init_server();
void* thread_session(void *param);
void send_file(int sock, char* filename, info_type *info, struct stat *sbuf);
void send_404(int sock);
int noprintf(const char *s, ...);

void parse_name_value(char* name, char* value, info_type* info)
{
  if(strcmp(name, "Content-Length") == 0){
    info->content_length = atoi(value);
  }else if(strcmp(name, "Referer") == 0){
    strcpy(info->referer, value);
  }
}

void parse_status_code(char* status, info_type* info)
{
  char cmd[1024];
  char path[1024];
  char version[1024];

  int j = 0;
  enum state_type
  {
    SEARCH_CMD,
    SEARCH_PATH,
    SEARCH_VERSION,
    FIN
  } state;

  state = SEARCH_CMD;

  for(unsigned int i = 0; i < strlen(status); i++){
    switch(state){
    case SEARCH_CMD:
      if(status[i] == ' '){
        cmd[j] = '\0';
        j = 0;
        state = SEARCH_PATH;
      }else{
        cmd[j] = status[i];
        j++;
      }
      break;
    case SEARCH_PATH:
      if(status[i] == ' '){
        path[j] = '\0';
        j = 0;
        state = SEARCH_VERSION;
      }else{
        path[j] = status[i];
        j++;
      }
      break;
    case SEARCH_VERSION:
      version[j] = status[i];
      j++;
      break;
    default:
      break;
    }
  }

  if(state != SEARCH_VERSION){
    printf("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
    exit(1);
  }

  strcpy(info->cmd, cmd);
  strcpy(info->path, path);

  if(strcmp(version, "HTTP/0.9") == 0){
    info->version = 0.9;
  }if(strcmp(version, "HTTP/1.0") == 0){
    info->version = 1.0;
  }else{
    info->version = 1.1;
  }

  int len = strlen(info->path);
  if(len > 5){
    if(strcmp(info->path + len - 5, ".html") == 0){
      info->content_type = TEXT_HTML;
    }else if(strcmp(info->path + len - 4, ".htm") == 0){
      info->content_type = TEXT_HTML;
    }else if(strcmp(info->path + len - 4, ".txt") == 0){
      info->content_type = TEXT_PLAIN;
    }else if(strcmp(info->path + len - 4, ".css") == 0){
      info->content_type = TEXT_CSS;
    }else{
      info->content_type = BINARY;
    }
  }

  DEBUG("info: %s, %s, %d, %f\n",
        info->cmd, info->path, info->content_length, info->version);
}

int parse_header(char* buf, int len, info_type *info)
{
  parse_state_type state = PARSE_STATUS;
  header_end_type status_end = SEARCH_END;
  char status[1024];
  char name[1024];
  char value[1024];
  int j = 0;
  int header_end_index = 0;

  for(int i = 0; i < len; i++){
    switch(status_end){
    case SEARCH_END:
      if(buf[i] == '\r'){
        status_end = CR;
      }else{
        status_end = SEARCH_END;
      }
      break;
    case CR:
      if(buf[i] == '\n'){
        status_end = CRLF;
      }else{
        status_end = SEARCH_END;
      }
      break;
    case CRLF:
      if(buf[i] == '\r'){
        status_end = CRLFCR;
      }else{
        status_end = SEARCH_END;
      }
      break;
    case CRLFCR:
      if(buf[i] == '\n'){
        status_end = CRLFCRLF;
        header_end_index = i;
      }else{
        status_end = SEARCH_END;
      }
      break;
    default:
      break;
    }

    switch(state){
      case PARSE_STATUS:
        if(buf[i] == '\r'){
          status[j] = '\0';
          j = 0;
          state = SKIP_LF;
          parse_status_code(status, info);
        }else{
          status[j] = buf[i];
          j++;
        }
        break;
      case PARSE_NAME:
        if(buf[i] == ':'){
          state = PARSE_VALUE;
          name[j] = '\0';
          j = 0;
        }else{
          name[j] = buf[i];
          j++;
        }
        break;
      case PARSE_VALUE:
        if(buf[i] == '\r'){
          state = SKIP_LF;
          value[j] = '\0';
          parse_name_value(name, value, info);
          j = 0;
        }else{
          value[j] = buf[i];
          j++;
        }
        break;
    case SKIP_LF:
      state = PARSE_NAME;
      break;
    default:
      break;
    }
  }

  DEBUG("PARSE_NAME = %d, state = %d\n", PARSE_NAME, state);
  DEBUG("CRLFCRLF = %d, status_end = %d\n", CRLFCRLF, status_end);
  DEBUG("header_end_index = %d, content_length = %d, len = %d\n",
        header_end_index, info->content_length, len);
  
  if(state == PARSE_NAME &&
     status_end == CRLFCRLF &&
     header_end_index + info->content_length + 1 >= len){
    return TRUE;
  }

  return FALSE;
}

void* thread_session(void *param){
  char buf[2048];
  struct stat sbuf;
  info_type info;

  bzero(&info, sizeof(info));

  thread_type *ttype = (thread_type*)param;
  int sock_client = ttype->sock;
  free(ttype);


  int tail = 0;
  while(1){
    int ret = recv(sock_client, buf + tail, 2048, 0);
    DEBUG("ret = %d\n", ret);
    DEBUG("%s\n", buf);

    if(ret <= 0){
      break;
    }

    tail = tail + ret;
    if(tail >= 2048){
      printf("%s:%d %s reading header\n", __FILE__, __LINE__, __FUNCTION__);
      exit(1);
    }

    int is_finished = parse_header(buf, tail, &info);
    if(is_finished == TRUE){
      break;
    }
  }

  char filename[1024];
  sprintf(filename, "html%s", info.path);
  DEBUG("filename = %s\n", filename);

  if(strcmp(info.path, "/") == 0){
    send_404(sock_client);
  }else if(strcmp(filename, "html") == 0){
    send_404(sock_client);
  }else if(strcmp(info.path, "/signal_sine") == 0){
    send_sinewave(sock_client);
  }else if(strcmp(info.path, "/signal_channel_a") == 0){
    send_channel_a(sock_client);
  }else if(strcmp(info.path, "/signal_channel_b") == 0){
    send_channel_b(sock_client);
  }else if(strcmp(info.path, "/signal_cross1_re") == 0){
    send_cross1_re(sock_client);
  }else if(strcmp(info.path, "/signal_cross1_im") == 0){
    send_cross1_im(sock_client);
  }else if(strcmp(info.path, "/signal_fft1_a") == 0){
    send_fft1_a(sock_client);
  }else if(strcmp(info.path, "/signal_fft1_b") == 0){
    send_fft1_b(sock_client);
  }else if(strcmp(info.path, "/signal_cross10_re") == 0){
    send_cross10_re(sock_client);
  }else if(strcmp(info.path, "/signal_cross10_im") == 0){
    send_cross10_im(sock_client);
  }else if(strcmp(info.path, "/signal_fft10_a") == 0){
    send_fft10_a(sock_client);
  }else if(strcmp(info.path, "/signal_fft10_b") == 0){
    send_fft10_b(sock_client);
  }else if(strcmp(info.path, "/signal_cross_total_re") == 0){
    send_cross_total_re(sock_client);
  }else if(strcmp(info.path, "/signal_cross_total_im") == 0){
    send_cross_total_im(sock_client);
  }else if(strcmp(info.path, "/signal_fft_total_a") == 0){
    send_fft_total_a(sock_client);
  }else if(strcmp(info.path, "/signal_fft_total_b") == 0){
    send_fft_total_b(sock_client);
  }else if(strcmp(info.path, "/cmd/reset") == 0){
    cmd_reset(sock_client, &info);
  }else if(strcmp(info.path, "/cmd/start") == 0){
    cmd_start(sock_client, &info);
  }else if(strcmp(info.path, "/cmd/stop") == 0){
    cmd_stop(sock_client, &info);
  }else if(strcmp(info.path, "/get/status") == 0){
    get_status(sock_client, &info);
  }else if(stat(filename, &sbuf) >= 0){
    send_file(sock_client, filename, &info, &sbuf);
  }else{
    send_404(sock_client);
  }

  if(shutdown(sock_client, 2) < 0){
    SDLAB_ERROR("shutdown");
  }

  close(sock_client);

  return NULL;
}

int init_server()
{
  char hostname[256];
  struct hostent *phost;
  struct sockaddr_in server_addr;
  int ret;

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0){
    SDLAB_ERROR("socket");
  }

  /* サーバホスト名取得 */
  ret = gethostname(hostname, sizeof(hostname));
  if(ret < 0){
    SDLAB_ERROR("gethostname");
  }
  printf("host name = %s\n", hostname);

  /* ホスト名からID取得 */
  phost = gethostbyname(hostname);
  if(phost == NULL){
    SDLAB_ERROR("gethostbyname");
  }

  printf("ip address lists\n");
  int n = 0;
  while(phost->h_addr_list[n] != NULL){
    struct in_addr *ia;
    ia = (struct in_addr *)phost->h_addr_list[n];
    printf("http://%s:%d/006_sdjnt.html\n", inet_ntoa(*ia), SERVER_PORT);
    n++;
  }
  printf("http://127.0.0.1:%d/006_sdjnt.html\n", SERVER_PORT);

  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  ret = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if(ret < 0){
    SDLAB_ERROR("bind");
  }

  ret = listen(sock, 5);
  if(ret < 0){
    SDLAB_ERROR("listen");
  }

  return sock;
}

void send_404(int sock)
{
  char buf[12048];

  sprintf(buf, "HTTP/1.0 404 Not Found\r\n\r\n");
  printf("%s", buf);
  if(send(sock, buf, strlen(buf), 0) < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send_404");
  }
}



void send_file(int sock, char* filename, info_type *info, struct stat *sbuf)
{
  char buf[12048];
  int len = 0;

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Server: sdlab_http/1.0\r\n");
  len += sprintf(buf + len, "Accept-Ranges: bytes\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", (int)sbuf->st_size);
  switch(info->content_type){
  case TEXT_HTML:
    len += sprintf(buf + len, "Content-Type: text/html\r\n");
    break;
  case TEXT_PLAIN:
    len += sprintf(buf + len, "Content-Type: text/plain\r\n");
    break;
  case TEXT_CSS:
    len += sprintf(buf + len, "Content-Type: text/css\r\n");
    break;
  default:
    len += sprintf(buf + len, "Content-Type: application/octet-stream\r\n");
    break;
  }

  len += sprintf(buf + len, "\r\n");
  printf("%s", buf);
  if(send(sock, buf, len, 0) < 0){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("send file header");
  }

  FILE *fp = fopen(filename, "r");
  if(fp == NULL){
    shutdown(sock, 2);
    close(sock);
    SDLAB_ERROR("fopen");
  }

  //ファイルの読み込みと送信
  len = fread(buf, sizeof(char), 1460, fp);
  while(len > 0){
    int ret;
    ret = send(sock, buf, len, 0);
    if(ret < 0){
      shutdown(sock, 2);
      close(sock);
      SDLAB_ERROR("send file content");
    }
    len = fread(buf, sizeof(char), 1460, fp);
  }

  fclose(fp);
}


void* sdlab_http_thread(void* param)
{
  int sock_server = init_server();
  signal(SIGPIPE , SIG_IGN);

  while(1){
    struct sockaddr_in client_addr;
    int sock_client;
    int len = sizeof(client_addr);
    thread_type *targ;

    sock_client = accept(sock_server, (struct sockaddr*)&client_addr,
                         (socklen_t*)&len);
//    printf("accept %d\n", sock_client);
    if(sock_client < 0){
      SDLAB_ERROR("accept");
    }

    targ = (thread_type*)malloc(sizeof(thread_type));
    targ->sock = sock_client;
    DEBUG("creating thread_session\n");

    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    pthread_attr_setstacksize(&tattr, 10 * 1024 * 1024);

    pthread_t th;
    pthread_create(&th, &tattr, thread_session, targ);
    pthread_detach(th);
    DEBUG("thraed detached\n");
  }

  return NULL;
}
