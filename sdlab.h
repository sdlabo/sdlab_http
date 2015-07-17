#ifndef SDLAB_H
#define SDLAB_H

#define SDLAB_PLOT_LEN 1024 * 100 //1 Mbyte
//#define SDLAB_PLOT_LEN 2 * 1024 * 1024

#define TRUE 1
#define FALSE 0

//#define DEBUG printf
#define DEBUG noprintf

enum enum_content_type {
  TEXT_HTML,
  TEXT_PLAIN,
  TEXT_CSS,
  BINARY
};

typedef struct
{
  char cmd[64];
  char path[256];
  char referer[256];
  int content_length;
  enum_content_type content_type;
  int code;
  double version;
} info_type;

extern int noprintf(const char *s, ...);

extern double sdlab_channel_a_buf[SDLAB_PLOT_LEN];
extern double sdlab_channel_b_buf[SDLAB_PLOT_LEN];

extern double sdlab_cross1_re_buf[SDLAB_PLOT_LEN];
extern double sdlab_cross1_im_buf[SDLAB_PLOT_LEN];
extern double sdlab_fft1_a_buf[SDLAB_PLOT_LEN];
extern double sdlab_fft1_b_buf[SDLAB_PLOT_LEN];

extern double sdlab_cross10_re_buf[SDLAB_PLOT_LEN];
extern double sdlab_cross10_im_buf[SDLAB_PLOT_LEN];
extern double sdlab_fft10_a_buf[SDLAB_PLOT_LEN];
extern double sdlab_fft10_b_buf[SDLAB_PLOT_LEN];

extern double sdlab_cross_total_re_buf[SDLAB_PLOT_LEN];
extern double sdlab_cross_total_im_buf[SDLAB_PLOT_LEN];
extern double sdlab_fft_total_a_buf[SDLAB_PLOT_LEN];
extern double sdlab_fft_total_b_buf[SDLAB_PLOT_LEN];

extern boost::mutex sdlab_mutex_channel_a;
extern boost::mutex sdlab_mutex_channel_b;
extern boost::mutex sdlab_mutex_cross1;
extern boost::mutex sdlab_mutex_cross10;
extern boost::mutex sdlab_mutex_cross_total;

extern int log_drop_count;
extern double log_calc_max;
extern double log_calc_min;
extern double log_calc_now;
extern double log_fft_max;
extern double log_fft_min;
extern double log_fft_now;

#endif
