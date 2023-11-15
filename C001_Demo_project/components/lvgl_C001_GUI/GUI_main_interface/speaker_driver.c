#include "speaker_driver.h"
#include "startup_music.h"
#include "driver/dac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_timer.h"
#include <sys/time.h>

#define NOP() asm volatile ("nop")

static unsigned long IRAM_ATTR micros()
{
    return (unsigned long) esp_timer_get_time();
}

/**
  * @brief  延时微秒
  * @param  us：微秒数
  * @retval void
  */
static void IRAM_ATTR delayMicroseconds(uint32_t us)
{
    uint32_t m = micros();
    if(us){
        uint32_t e = (m + us);
        if(m > e){ //overflow
            while(micros() > e){
                NOP();
            }
        }
        while(micros() < e){
            NOP();
        }
    }
}

/**
  * @brief  播放开机音乐
  * @param  void
  * @retval void
  */
void Speaker_Play_Seartup(void)
{
    Speaker_Play_Music(Startup_music, 25);
}

/**
  * @brief  播放音乐
  * @param  music_data：音乐数据
  * @param  rate：dac频率
  * @retval void
  */
void Speaker_Play_Music(const unsigned char* music_data, int rate)
{
    uint32_t length = 15000;
    // printf("length = %d.\n",length);
    for(int i=0; i<length; i++){
        dac_output_enable(SPEAKER_DAC_NUM);
        if(music_data[i] > 130){
            dac_output_voltage(SPEAKER_DAC_NUM, music_data[i]*0.8);
        }else{
            dac_output_voltage(SPEAKER_DAC_NUM, music_data[i]*0.3);
        }
            
        dac_output_disable(SPEAKER_DAC_NUM);
        delayMicroseconds(rate);
    }
}

