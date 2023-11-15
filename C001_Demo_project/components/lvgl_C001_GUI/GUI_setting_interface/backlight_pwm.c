#include <stdio.h>
#include "backlight_pwm.h"
#include "driver/ledc.h"
#include "esp_err.h"

static ledc_channel_config_t *led_pwm_channel = NULL;

/**
  * @brief  初始化背光ledpwm
  * @param  void
  * @retval void
  */
void Backlight_Pwm_Init(void)
{   
    //ledc定时器结构体配置
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);                 //设置ledc定时器
    led_pwm_channel = (ledc_channel_config_t *)malloc(sizeof(ledc_channel_config_t));
    //pwm通道0配置
    led_pwm_channel->channel = LEDC_CHANNEL_0;
    led_pwm_channel->duty = 0;
    led_pwm_channel->gpio_num = Backlight_PWM;
    led_pwm_channel->speed_mode = LEDC_HIGH_SPEED_MODE;
    led_pwm_channel->timer_sel = LEDC_TIMER_0;
    // led_pwm_channel->hpoint = 0;
    ledc_channel_config(led_pwm_channel);           //配置PWM
}   

/**
  * @brief  设置背光值
  * @param  void
  * @retval void
  * @note   改变LED亮度百分比（LED的PWM占空比）不要超过100
  */
void Change_Backlight_pwm(uint32_t PWM_Percentage)
{
    if(PWM_Percentage > 100){
        return;
    }
    ledc_set_duty(led_pwm_channel->speed_mode, 
                  led_pwm_channel->channel, 
                  PWM_Percentage*81);

    ledc_update_duty(led_pwm_channel->speed_mode, 
                     led_pwm_channel->channel);
}


