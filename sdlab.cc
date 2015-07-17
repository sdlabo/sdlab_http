#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "sdlab_http.h"
#include "sdlab_signal.h"

void create_http_thread(pthread_t *p_th)
{
  pthread_create(p_th, NULL, sdlab_http_thread, NULL);
  printf("http_thraed is created\n");
}

void create_signal_thread(pthread_t *p_th)
{
  pthread_create(p_th, NULL, sdlab_signal_thread, NULL);
  printf("signal_thraed is created\n");
}


int main(int argc, char **argv)
{
  pthread_attr_t tattr;
  size_t size;
  int ret;

/* スタックの大きさを取得する */
  pthread_attr_init(&tattr);
  ret = pthread_attr_getstacksize(&tattr, &size);
  printf("stack size = %d\n", size);
  printf("ret = %d\n", ret);

  ret = pthread_attr_getguardsize(&tattr, &size);
  printf("guard size = %d\n", size);
  printf("ret = %d\n", ret);

  ret = pthread_attr_setstacksize(&tattr, 1000 * 1000);
  ret = pthread_attr_getstacksize(&tattr, &size);
  printf("stack size = %d\n", size);
  printf("ret = %d\n", ret);

  ret = pthread_attr_setguardsize(&tattr, 10 * 1000 * 1000);
  ret = pthread_attr_getguardsize(&tattr, &size);
  printf("guard size = %d\n", size);
  printf("ret = %d\n", ret);
  

//  exit(-1);

  pthread_t http_thread, signal_thread;
  create_http_thread(&http_thread);
  create_signal_thread(&signal_thread);

  pthread_join(http_thread, NULL);
  pthread_join(signal_thread, NULL);

  return 0;
}
