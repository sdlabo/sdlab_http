#include <stdlib.h>
#include <stdio.h>
#include "sdlab_http.h"
#include "sdlab_udpfft.h"

#include <boost/thread.hpp>

int main(int argc, char **argv)
{
  boost::thread thread_http(&sdlab_http_thread);
  boost::thread thread_udpfft(&sdlab_udpfft_thread);

  boost::posix_time::time_duration timeout =
    boost::posix_time::milliseconds(500);
  
  int ret = false;
  while(ret == false){
    ret = thread_http.timed_join(timeout);
  }

  ret = false;
  while(ret == false){
    thread_udpfft.timed_join(timeout);
  }

  return 0;
}
