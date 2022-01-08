#include <stdint.h>

#define SAMPLE_MIN         2
#define SAMPLE_CENTER    517
#define SAMPLE_MAX      1032

#ifdef __32X__
#define SND_ATTR_SDRAM  __attribute__((section(".data"), aligned(16)))
#else
#define SND_ATTR_SDRAM 
#endif

void snddma_submit(void) SND_ATTR_SDRAM;
uint16_t* snddma_get_buf(int channels, int num_samples) SND_ATTR_SDRAM;
uint16_t* snddma_get_buf_mono(int num_samples) SND_ATTR_SDRAM;
uint16_t* snddma_get_buf_stereo(int num_samples) SND_ATTR_SDRAM;

static inline uint16_t s16pcm_to_u16pwm(int16_t s) {
    s = (s >> 5) + SAMPLE_CENTER;
    return (s < 0) ? SAMPLE_MIN : (s > SAMPLE_MAX) ? SAMPLE_MAX : s;
}

void snddma_slave_init(int sample_rate);
void snddma_init(int sample_rate);
void slave_dma_kickstart(void);
unsigned snddma_length(void)SND_ATTR_SDRAM;
void snddma_wait(void) SND_ATTR_SDRAM;
