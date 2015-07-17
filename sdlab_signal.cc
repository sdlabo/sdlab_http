#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <pthread.h>

#include "UDPComm.h"
#include "fftsg.h"
#include "sdlab.h"

#define TMPDATA_BASE "/tmp/sdlab" // the base directory to store result data
#define DATA_SIZE (2*1024*1024)  // 2^(21) points

#define PORTA (0x4000) // receive port #.
#define PORTB (0x4001) // receive port #.

// the size of a buffer for receiving a UDP packet
#define RECV_BUF_SIZE   (2048)
// the size of a length of data in a UDP packet
#define DATA_BURST_SIZE  (512)

// the number of data stored in a file
//#define DATA_PER_FILE   (100) 
// the time to write the data into the file per computations.
//#define RESULT_SAVE_TIME (20) 

// the number of data stored in a file
#define DATA_PER_FILE   (5)
// the time to write the data into the file per computations.
#define RESULT_SAVE_TIME (5)
// the time to create new file per saving.
#define CREATE_NEW_FILE_TIME (DATA_PER_FILE/RESULT_SAVE_TIME)
// the buffer size more than 2 has not been supported yet
#define NUM_OF_ENV_BUFFER (2)

// a working data for receiving packet and FFT operation
struct env{
  // a pointer to the instance of UDP communication
  UDPComm *comm;
  // receiving data buffers, selecting as double buffer
  double  *buf[NUM_OF_ENV_BUFFER];
  // a pointer to the active buffer to receive UDP packet.
  double  *cur;
  // a pointer to the buffer working FFT operation
  double  *fft_work; 
  // internal data for FFT operation
  int     *ip;
  // internal data for FFT operation
  double  *w;
  // temporal buffer to receive a UDP packet.
  char recv_buf[RECV_BUF_SIZE];
  // identifier for the active buffer;
  int     buf_id;
};

double data_fft10_a[DATA_SIZE];
double data_fft10_b[DATA_SIZE];
double data_cross10_re[DATA_SIZE];
double data_cross10_im[DATA_SIZE];

double data_fft_total_a[DATA_SIZE];
double data_fft_total_b[DATA_SIZE];
double data_cross_total_re[DATA_SIZE];
double data_cross_total_im[DATA_SIZE];

int result_total;
int save_flag;
int log_drop_count;
double log_calc_max = DBL_MIN;
double log_calc_min = DBL_MAX;
double log_calc_now;
double log_fft_max = DBL_MIN;
double log_fft_min = DBL_MAX;
double log_fft_now;

// a system envrionment
struct system_env{
  char file_postfix[128]; // postfix of file to stored result data
  int file_postfix_count; // postfix #. of file to stored result data
  // result buffers to save results into HDD in a group, used as double buffer
  double * result[2];
  double * cur; // a pointer to the active buffer to pack result
  double * dump; // a pointer to the buffer to save HDD
  int result_count; // the number of buffering results
  int dump_count; // the number of results saving in current opening file
};

void *fft_thread(void *param); // FFT wrapper function.

static void * malloc_or_die(size_t sz, const char *msg)
{
  void* ptr = malloc(sz);
  if(ptr == NULL){
    fprintf(stderr, "%s\n", msg);
    exit(0);
  }
  return ptr;
}

static void sys_init(struct system_env *sys){

  // postfilx of file to stored result data is decided based on current time.
  time_t now;
  struct tm *ts;
  now = time(NULL);
  ts = localtime(&now);
  strftime(sys->file_postfix, sizeof(sys->file_postfix), "%Y%m%d_%H%M%S", ts);

  // buffers are allocated
  sys->result[0] = (double*)malloc_or_die(
    sizeof(double) * DATA_SIZE * 4 / 2,
    "cannot allocate save result buffer (0)");
  sys->result[1] = (double*)malloc_or_die(
    sizeof(double) * DATA_SIZE * 4 / 2,
    "cannot allocate save result buffer (1)");

  sys->result_count = 0;
  sys->dump_count = 0;
  sys->file_postfix_count = 0;
  sys->cur = sys->result[0];
}

void sys_free(struct system_env *sys){
  free(sys->result[0]);
  free(sys->result[1]);
}

void env_init(struct env *e, int port){
  // required memories are allocated
  for(int i = 0; i < NUM_OF_ENV_BUFFER; i++){
    e->buf[i] = (double*)malloc_or_die(
      sizeof(double) * DATA_SIZE * 2,
      "cannot allocate heap memory for I/O data");
  }
  e->ip = (int*)malloc_or_die(
    sizeof(int) * (2 + sqrt(DATA_SIZE * 2)),
    "cannot allocate heap memory for bit reversal");
  e->w  = (double*)malloc_or_die(
    sizeof(double) * DATA_SIZE,
    "cannot allocate heap memory for cos/sin table");

  // poiters are initialized.
  e->buf_id   = 0;
  e->cur      = e->buf[e->buf_id];
  e->fft_work = e->buf[e->buf_id];

  // to make the constant table for FFT before actual operation.
  e->ip[0] = 0;
  fft_thread(e);

  // open socket to receive UDP packets
  e->comm = new UDPComm(INADDR_ANY, port);
}

void env_free(struct env *e){
  free(e->buf[0]);
  free(e->buf[1]);
  free(e->ip);
  free(e->w);
  delete(e->comm);
}

void *fft_thread(void *param)
{
  struct env *e = (struct env*) param;
  cdft(DATA_SIZE, -1, e->fft_work, e->ip, e->w);

  return NULL;
}


//static void fft(struct env *e){
//  cdft(DATA_SIZE, -1, e->fft_work, e->ip, e->w);
//}

struct calc_arg{
  struct system_env *sys;
  struct env *e0; // pointer for F(a)
  struct env *e1; // pointer for F(b)
};

/**
 * save results of F(a) and F(b) into the internal buffer.
 * data format in the internal buffer is as fowllows;
 *  - cross-Re
 *  - cross-Im
 *  - F(a)-Re
 *  - F(b)-Re
 */
void save_result(struct calc_arg *arg){
  double a, b, c, d;
  double x_re[DATA_SIZE / 2];
  double x_im[DATA_SIZE / 2];
  double p_a[DATA_SIZE / 2];
  double p_b[DATA_SIZE / 2];

  for(int i = 0; i < DATA_SIZE / 2; i++){
    a = arg->e0->fft_work[2 * i];   // F(a)-Re
    b = arg->e0->fft_work[2 * i + 1]; // F(a)-Im
    c = arg->e1->fft_work[2 * i];   // F(b)-Re
    d = arg->e1->fft_work[2 * i + 1]; // F(b)-Im
    x_re[i] = a * c + b * d;
    x_im[i] = b * c - a * d;
    p_a[i] = a * a + b * b;
    p_b[i] = c * c + d * d;
    arg->sys->cur[DATA_SIZE / 2 * 0 + i] += x_re[i];
    arg->sys->cur[DATA_SIZE / 2 * 1 + i] += x_im[i];
    arg->sys->cur[DATA_SIZE / 2 * 2 + i] += p_a[i];
    arg->sys->cur[DATA_SIZE / 2 * 3 + i] += p_b[i];
  }


  {
    result_total++;
    for(int i = 0; i < SDLAB_PLOT_LEN; i++){
      sdlab_cross1_re_buf[i] = x_re[i];
      sdlab_cross1_im_buf[i] = x_im[i];
      sdlab_fft1_a_buf[i] = p_a[i];
      sdlab_fft1_b_buf[i] = p_b[i];

      data_cross10_re[i] = arg->sys->cur[DATA_SIZE / 2 * 0 + i];
      data_cross10_im[i] = arg->sys->cur[DATA_SIZE / 2 * 1 + i];
      data_fft10_a[i] = arg->sys->cur[DATA_SIZE / 2 * 2 + i];
      data_fft10_b[i] = arg->sys->cur[DATA_SIZE / 2 * 3 + i];

      data_cross_total_re[i] += x_re[i];
      sdlab_cross_total_re_buf[i] = data_cross_total_re[i] /
        ((double)result_total);

      data_cross_total_im[i] += x_im[i];
      sdlab_cross_total_im_buf[i] = data_cross_total_im[i] /
        ((double)result_total);

      data_fft_total_a[i] += p_a[i];
      sdlab_fft_total_a_buf[i] = data_fft_total_a[i] /
        ((double)result_total);

      data_fft_total_b[i] += p_b[i];
      sdlab_fft_total_b_buf[i] = data_fft_total_b[i] /
        ((double)result_total);
    }
  }

  // increment the counter for the number of stored results. 
  arg->sys->result_count++; 
}

/**
 * save results stored in the internal buffer into HDD in a group.
 */
//void dump_result_to_file(struct calc_arg *arg){
void* dump_thread(void *param){
  struct calc_arg *arg = (struct calc_arg*) param;
  // make file name and open the file
  char str[256];
  sprintf(str, "%s_%s_%08d_%d.data",
          TMPDATA_BASE, arg->sys->file_postfix, arg->sys->file_postfix_count,
          RESULT_SAVE_TIME);

  printf("opening %s\n", str);
  FILE *fp;
  if(arg->sys->dump_count == 0){
    // the file is created newly at first.
    printf("create: %s\n", str);
    fp = fopen(str, "wb");
  }else{
    // otherwise, the file is open in append mode.
    printf("open: %s (%d)\n", str, arg->sys->dump_count);
    fp = fopen(str, "ab");
  }
  if(fp == NULL){
    fprintf(stderr, "cannot open file: %s", str);
    exit(0);
  }

  // write all data in the internal buffer
  fwrite(arg->sys->dump, sizeof(double), 4 * DATA_SIZE / 2, fp);
  fflush(fp);
  fclose(fp);
  bzero(arg->sys->dump, sizeof(double) * 4 * DATA_SIZE / 2);

  arg->sys->dump_count++;
  // counter is cleared every CREATE_NEW_FILE_TIME times.
  if(arg->sys->dump_count == CREATE_NEW_FILE_TIME){
    arg->sys->dump_count = 0;
    arg->sys->file_postfix_count++;
  }

  printf("\t\t\tfile save done\n");

  return NULL;
}

/**
 * FFT operation for received data.
 */
//void calc(struct calc_arg *arg){
void *calc_thread(void *param)
{
  struct calc_arg *arg = (struct calc_arg*) param;
  clock_t start = clock();
  pthread_t fft0, fft1;

  pthread_create(&fft0, NULL, fft_thread, arg->e0);
  pthread_create(&fft1, NULL, fft_thread, arg->e1);

  pthread_join(fft0, NULL);
  pthread_join(fft1, NULL);

  clock_t fft_end = clock();
  log_fft_now = ((double)(fft_end - start)) / CLOCKS_PER_SEC;
  if(log_fft_max < log_fft_now){
    log_fft_max = log_fft_now;
  }

  if(log_fft_min > log_fft_now){
    log_fft_min = log_fft_now;
  }

  save_result(arg);

  if(arg->sys->result_count == RESULT_SAVE_TIME){
    arg->sys->result_count = 0;
    arg->sys->dump = arg->sys->cur; // to point the buffer keeping results.
    arg->sys->cur = (arg->sys->cur == arg->sys->result[0]) ?
      arg->sys->result[1] : arg->sys->result[0];
    printf("\t\tkick save\n");

    if(save_flag == TRUE){
      printf("saving file\n");
      pthread_t th;
      pthread_create(&th, NULL, dump_thread, arg);
      pthread_detach(th);
    }

    {
      for(int i = 0; i < SDLAB_PLOT_LEN; i++){
        sdlab_cross10_re_buf[i] = data_cross10_re[i] / 10.0;
        sdlab_cross10_im_buf[i] = data_cross10_im[i] / 10.0;
        sdlab_fft10_a_buf[i] = data_fft10_a[i] / 10.0;
        sdlab_fft10_b_buf[i] = data_fft10_b[i] / 10.0;
      }
    }

    bzero((void*) data_cross10_re, sizeof(double) * DATA_SIZE);
    bzero((void*) data_cross10_im, sizeof(double) * DATA_SIZE);
    bzero((void*) data_fft10_a, sizeof(double) * DATA_SIZE);
    bzero((void*) data_fft10_b, sizeof(double) * DATA_SIZE);
  }
  printf("\tcalc done\n");
  clock_t calc_end = clock();

  log_calc_now = ((double)(calc_end - start)) / CLOCKS_PER_SEC;
  if(log_calc_max < log_calc_now){
    log_calc_max = log_calc_now;
  }

  if(log_calc_min > log_calc_now){
    log_calc_min = log_calc_now;
  }

  return NULL;
}

/**
 * Receive UDP packets:
 *  Each UDP packet contains DATA_BURST_SIZE data.
 *  All packets for FFT (DATA_SIZE) is received in this function.
 */
void *recv_thread(void *param){
  struct env *e = (struct env*) param;
  int idx = 0;
  int id = 0, prev_id = 0;
  while(idx < DATA_SIZE){
    e->comm->data_recv(e->recv_buf, sizeof(int) * RECV_BUF_SIZE);
//    id = ntohl(*(int*)(e->recv_buf));
    int *p = (int*) e->recv_buf;
    id = ntohl(*p);
    if(idx == 0 && id != 0){
      // drop packets until receiving a start packet.
      continue;
    }

    if((id - prev_id) > DATA_BURST_SIZE){
      printf("!!! Drop a packet: %08x - %08x = %08x\n",
             id, prev_id, id - prev_id);
      log_drop_count++;
    }

    prev_id = id;
    short *data = (short*)(e->recv_buf + sizeof(int));
    for(int i = 0; i < DATA_BURST_SIZE; i++){
      unsigned short s = ntohs(data[i]);
      e->cur[2 * (idx + i) + 0] = ((double) s); // Re
      e->cur[2 * (idx + i) + 1] = (double) 0; // Im
    }
    idx += DATA_BURST_SIZE;
  }

  if(e->comm->get_port() == 0x4000){
    for(int i = 0; i < SDLAB_PLOT_LEN; i++){
      sdlab_channel_a_buf[i] = e->cur[2 * i];
    }
  }else{
    for(int i = 0; i < 1000; i++){
      sdlab_channel_b_buf[i] = e->cur[2 * i];
    }
  }

  return NULL;
}

/**
 * swap the working buffer in round-robin manner.
 */
void swap_buffer(struct env *e){
  e->fft_work = e->cur;
  e->buf_id = (e->buf_id == NUM_OF_ENV_BUFFER-1) ? 0 : e->buf_id + 1;
  e->cur = e->buf[e->buf_id];
}

void sdlab_udpfft_reset()
{
  bzero((void*) data_cross_total_re, sizeof(double) * DATA_SIZE);
  bzero((void*) data_cross_total_im, sizeof(double) * DATA_SIZE);
  bzero((void*) data_fft_total_a, sizeof(double) * DATA_SIZE);
  bzero((void*) data_fft_total_b, sizeof(double) * DATA_SIZE);

  result_total = 0;
}

void sdlab_start_save()
{
  save_flag = TRUE;
}

void sdlab_stop_save()
{
  save_flag = FALSE;
}

int sdlab_get_drop_count()
{
  return log_drop_count;
}

void sdlab_reset_drop_count()
{
  log_drop_count = 0;
}

void* sdlab_signal_thread(void *param)
{
  struct system_env sys;
  struct env e0, e1;   // working data structures for F(a) and F(b)
  struct calc_arg arg;

  save_flag = FALSE;

  bzero((void*) data_cross10_re, sizeof(double) * DATA_SIZE);
  bzero((void*) data_cross10_im, sizeof(double) * DATA_SIZE);
  bzero((void*) data_fft10_a, sizeof(double) * DATA_SIZE);
  bzero((void*) data_fft10_b, sizeof(double) * DATA_SIZE);

  bzero((void*) data_cross_total_re, sizeof(double) * DATA_SIZE);
  bzero((void*) data_cross_total_im, sizeof(double) * DATA_SIZE);
  bzero((void*) data_fft_total_a, sizeof(double) * DATA_SIZE);
  bzero((void*) data_fft_total_b, sizeof(double) * DATA_SIZE);

  result_total = 0;
  log_drop_count = 0;

  sys_init(&sys);
  env_init(&e0, PORTA);
  env_init(&e1, PORTB);

  arg.sys = &sys;
  arg.e0 = &e0;
  arg.e1 = &e1;

  while(1){
    // receive UDP packets for F(a) and F(b)
    pthread_t th0, th1;
    
    pthread_create(&th0, NULL, recv_thread, &e0);
    pthread_create(&th1, NULL, recv_thread, &e1);

    pthread_join(th0, NULL);
    pthread_join(th1, NULL);
    
    
//    boost::thread recv_thread0(&recv_data, &e0);
//    boost::thread recv_thread1(&recv_data, &e1);
    // wait for receiving UDP pcakets.


    swap_buffer(&e0);
    swap_buffer(&e1);

    // do FFT operation
    printf("kick calc\n");

    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    pthread_attr_setstacksize(&tattr, 256 * 1024 * 1024);

    pthread_t th;
    pthread_create(&th, &tattr, calc_thread, &arg);
    pthread_detach(th);
  }

  env_free(&e0);
  env_free(&e1);
  sys_free(&sys);

  return NULL;
}
