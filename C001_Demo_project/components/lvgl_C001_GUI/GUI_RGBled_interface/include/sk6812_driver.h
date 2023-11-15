#ifndef _SK6812_DRIVER_H
#define _SK6812_DRIVER_H
#include "esp_err.h"

#define SK6812_GPIO         5               //SK6812io脚号

#define SK6812_GPIO_HIG     gpio_set_level(SK6812_GPIO, 1);  //设置SK6812io脚输出高电平
#define SK6812_GPIO_LOW     gpio_set_level(SK6812_GPIO, 0);  //设置SK6812io脚输出低电平

//配合delay_ns使用，大概的延时时间
#define DELAY_NS_800000     4000
#define DELAY_NS_300        1
#define DELAY_NS_900        6
#define DELAY_NS_600        3

/**
  * @brief  初始化SK6812gpio
  * @param  void
  * @retval void
  */
esp_err_t SK6812_Init(void);

/**
  * @brief  SK6812写一个颜色数据
  * @param  red : 红色数据
  * @param  green : 绿色数据
  * @param  blue : 蓝色数据
  * @retval void
  */
void SK6812_Set_RGBColor(uint8_t green, uint8_t red, uint8_t blue);

/**
  * @brief  SK6812渐变色
  * @param  time ： 渐变时间单位，推荐1000-10000
  * @retval void
  */
void SK6812_Gradient_Color(uint32_t time);

#endif /* _SK6812_DRIVER_H */