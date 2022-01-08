#include <stdint.h>
#include "32x.h"
#include "sound.h"
#include "mars_ringbuf.h"

marsrb_t soundbuf;

void slave_dma1_handler(void) SND_ATTR_SDRAM;
void slave_dma_kickstart(void) SND_ATTR_SDRAM;

uint16_t* snddma_get_buf(int channels, int num_samples) {
    uint16_t* p;

    p = (uint16_t*)Mars_RB_GetWriteBuf(&soundbuf, 8);
    if (!p)
        return NULL;
    *p++ = channels;
    *p++ = num_samples;
    Mars_RB_CommitWrite(&soundbuf);

    return (uint16_t*)Mars_RB_GetWriteBuf(&soundbuf, num_samples * channels);
}

uint16_t* snddma_get_buf_mono(int num_samples) {
    return snddma_get_buf(1, num_samples);
}

uint16_t* snddma_get_buf_stereo(int num_samples) {
    return snddma_get_buf(2, num_samples);
}

void snddma_wait(void) {
    Mars_RB_WaitReader(&soundbuf, 0);
}

void snddma_submit(void) {
    Mars_RB_CommitWrite(&soundbuf);
}

unsigned snddma_length(void)
{
    return Mars_RB_Len(&soundbuf);
}

void slave_dma_kickstart(void)
{
    static short kickstart_samples[16] __attribute__((aligned(16))) = {
        SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER,
        SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER
    };

    SH2_DMA_SAR1 = (intptr_t)kickstart_samples;
    SH2_DMA_TCR1 = sizeof(kickstart_samples) / sizeof(kickstart_samples[0]);
    SH2_DMA_DAR1 = 0x20004038; // storing a word here will the MONO channel
    SH2_DMA_CHCR1 = 0x14e5; // dest fixed, src incr, size word, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr enabled, clear TE, dma enabled
}

void sec_dma1_handler(void)
{
    //Mars_RB_CommitRead(&soundbuf);

    SH2_DMA_CHCR1; // read TE
    SH2_DMA_CHCR1 = 0; // clear TE

    //if (Mars_RB_Len(&soundbuf) == 0)
    {
        // sound buffer UNDERRUN
        //slave_dma_kickstart();
        return;
    }

    short* p = Mars_RB_GetReadBuf(&soundbuf, 8);
    int num_channels = *p++;
    int num_samples = *p++;
    Mars_RB_CommitRead(&soundbuf);

    p = Mars_RB_GetReadBuf(&soundbuf, num_samples * num_channels);

    SH2_DMA_SAR1 = (intptr_t)p | 0x20000000;
    SH2_DMA_TCR1 = num_samples;
    if (num_channels == 2)
    {
        SH2_DMA_DAR1 = 0x20004034; // storing a long here will set left and right
        SH2_DMA_CHCR1 = 0x18e5; // dest fixed, src incr, size long, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr enabled, clear TE, dma enabled
    }
    else
    {
        SH2_DMA_DAR1 = 0x20004038; // storing a word here will set the MONO channel
        SH2_DMA_CHCR1 = 0x14e5; // dest fixed, src incr, size word, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr enabled, clear TE, dma enabled
    }
}

void snddma_slave_init(int sample_rate)
{
    uint16_t sample, ix;

   // Mars_RB_ResetRead(&soundbuf);

    // init DMA
    SH2_DMA_SAR0 = 0;
    SH2_DMA_DAR0 = 0;
    SH2_DMA_TCR0 = 0;
    SH2_DMA_CHCR0 = 0;
    SH2_DMA_DRCR0 = 0;
    SH2_DMA_SAR1 = 0;
    SH2_DMA_DAR1 = 0x20004038; // storing a word here will the MONO channel
    SH2_DMA_TCR1 = 0;
    SH2_DMA_CHCR1 = 0;
    SH2_DMA_DRCR1 = 0;
    SH2_DMA_DMAOR = 1; // enable DMA

    SH2_DMA_VCR1 = 72; // set exception vector for DMA channel 1
    SH2_INT_IPRA = (SH2_INT_IPRA & 0xF0FF) | 0x0F00; // set DMA INT to priority 15
    
    // init the sound hardware
    MARS_PWM_MONO = 1;
    MARS_PWM_MONO = 1;
    MARS_PWM_MONO = 1;
    if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
        MARS_PWM_CYCLE = (((23011361 << 1) / (sample_rate) + 1) >> 1) + 1; // for NTSC clock
    else
        MARS_PWM_CYCLE = (((22801467 << 1) / (sample_rate) + 1) >> 1) + 1; // for PAL clock
    MARS_PWM_CTRL = 0x0185; // TM = 1, RTP, RMD = right, LMD = left

    sample = SAMPLE_MIN;

    // ramp up to SAMPLE_CENTER to avoid click in audio (real 32X)
    while (sample < SAMPLE_CENTER)
    {
        for (ix = 0; ix < (sample_rate * 2) / (SAMPLE_CENTER - SAMPLE_MIN); ix++)
        {
            while (MARS_PWM_MONO & 0x8000); // wait while full
            MARS_PWM_MONO = sample;
        }
        sample++;
    }

    slave_dma_kickstart();

    SetSH2SR(2);
}

void snddma_init(int sample_rate)
{
    //Mars_RB_ResetAll(&soundbuf);
}
