#ifndef _SPEAKER_DRIVER_
#define _SPEAKER_DRIVER_

#define SPEAKER_DAC_NUM         DAC_CHANNEL_1
#define SPEAKER_IO_NUM          25

/**
  * @brief  播放开机音乐
  * @param  void
  * @retval void
  */
void Speaker_Play_Seartup(void);

/**
  * @brief  播放音乐
  * @param  music_data：音乐数据
  * @param  rate：dac频率
  * @retval void
  */
void Speaker_Play_Music(const unsigned char* music_data, int rate);

#endif /* _SPEAKER_DRIVER_ */