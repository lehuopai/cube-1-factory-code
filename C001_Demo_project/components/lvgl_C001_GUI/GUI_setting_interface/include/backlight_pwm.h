#ifndef __BACKLIGHT_PWM_DRIVER_H
#define __BACKLIGHT_PWM_DRIVER_H

#define Backlight_PWM                32          //led_gpio引脚号

/**
  * @brief  初始化背光ledpwm
  * @param  void
  * @retval void
  */
void Backlight_Pwm_Init(void);

/**
  * @brief  设置背光值
  * @param  void
  * @retval void
  * @note   改变LED亮度百分比（LED的PWM占空比）不要超过100
  */
void Change_Backlight_pwm(uint32_t PWM_Percentage);

#endif /* __BACKLIGHT_PWM_DRIVER_H */