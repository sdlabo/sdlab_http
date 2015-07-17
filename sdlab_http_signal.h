#ifndef SDLAB_HTTP_SIGNAL_H
#define SDLAB_HTTP_SIGNAL_H

extern int sprintf_sinewave(char* buf, double amp, double phase, int num);

extern int sprintf_channel_a(char* buf, double amp, int num);
extern int sprintf_channel_b(char* buf, double amp, int num);
extern int sprintf_cross1_re(char* buf, double amp, int num);
extern int sprintf_cross1_im(char* buf, double amp, int num);
extern int sprintf_fft1_a(char* buf, double amp, int num);
extern int sprintf_fft1_b(char* buf, double amp, int num);
extern int sprintf_cross10_re(char* buf, double amp, int num);
extern int sprintf_cross10_im(char* buf, double amp, int num);
extern int sprintf_fft10_a(char* buf, double amp, int num);
extern int sprintf_fft10_b(char* buf, double amp, int num);
extern int sprintf_cross_total_re(char* buf, double amp, int num);
extern int sprintf_cross_total_im(char* buf, double amp, int num);
extern int sprintf_fft_total_a(char* buf, double amp, int num);
extern int sprintf_fft_total_b(char* buf, double amp, int num);


extern void send_sinewave(int sock);
extern void send_channel_a(int sock);
extern void send_channel_b(int sock);

extern void send_cross1_re(int sock);
extern void send_cross1_im(int sock);
extern void send_fft1_a(int sock);
extern void send_fft1_b(int sock);

extern void send_cross10_re(int sock);
extern void send_cross10_im(int sock);
extern void send_fft10_a(int sock);
extern void send_fft10_b(int sock);

extern void send_cross_total_re(int sock);
extern void send_cross_total_im(int sock);
extern void send_fft_total_a(int sock);
extern void send_fft_total_b(int sock);

#endif
