#include "sk6812_driver.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/timer.h"
#include "esp_log.h"

const char TAG[] = "SK6812";

/**
  * @brief  初始化SK6812gpio
  * @param  void
  * @retval void
  */
esp_err_t SK6812_Init(void)
{
    esp_err_t err;
    gpio_pad_select_gpio(SK6812_GPIO);
    err = gpio_set_direction(SK6812_GPIO, GPIO_MODE_OUTPUT);
    if(err == ESP_FAIL){
        ESP_LOGE(TAG, "SK6812 init gpio failed.\n");
        return ESP_FAIL;
    }
    SK6812_GPIO_HIG;
    return ESP_OK;
}

/**
  * @brief  初始化SK6812gpio
  * @param  void
  * @retval void
  */
static void SK6812_Delay_ns(uint32_t ns)
{
    volatile uint32_t i=0;
    for(i=0; i<ns; i++);
}

/**
  * @brief  SK6812复位
  * @param  void
  * @retval void
  */
static void SK6812_Reset(void)
{
    SK6812_GPIO_LOW;
    SK6812_Delay_ns(DELAY_NS_800000);
}

/**
  * @brief  写0
  * @param  void
  * @retval void
  */
static void SK6812_Write_0(void)
{
	SK6812_GPIO_HIG;
    SK6812_Delay_ns(DELAY_NS_300);
	SK6812_GPIO_LOW;
    SK6812_Delay_ns(DELAY_NS_900);
}

/**
  * @brief  写1
  * @param  void
  * @retval void
  */
static void SK6812_Write_1(void)
{
	SK6812_GPIO_HIG;
    SK6812_Delay_ns(DELAY_NS_600);
	SK6812_GPIO_LOW;
    SK6812_Delay_ns(DELAY_NS_600);
}

/**
  * @brief  SK6812写一个数据
  * @param  byte : 待写的数据
  * @retval void
  */
static void SK6812_Write_Byte(uint8_t byte)
{
    uint8_t i;
    for(i=0;i<8;i++){
        if(byte&0x80){
		    SK6812_Write_1();
	    }else{
		    SK6812_Write_0();
	    }
		byte <<= 1;
    }
}

/**
  * @brief  SK6812写一个颜色数据
  * @param  red : 红色数据
  * @param  green : 绿色数据
  * @param  blue : 蓝色数据
  * @retval void
  */
void SK6812_Set_RGBColor(uint8_t red, uint8_t green, uint8_t blue)
{
  SK6812_Reset();
  SK6812_Write_Byte(green);
	SK6812_Write_Byte(red);
	SK6812_Write_Byte(blue);
}

/**
  * @brief  SK6812渐变色
  * @param  time ： 渐变时间单位，推荐1000-10000
  * @retval void
  */
void SK6812_Gradient_Color(uint32_t time)
{
    uint8_t i=0;
    uint8_t j=0;
    uint8_t k=255;
    for(i=0; i<255; i++){
        k--;
        SK6812_Set_RGBColor(i, j, k);
        SK6812_Delay_ns(time);
    }
    for(j=0; j<255; j++){
        i--;
        SK6812_Set_RGBColor(i, j, k);
        SK6812_Delay_ns(time);
    }
    for(k=0; k<255; k++){
        j--;
        SK6812_Set_RGBColor(i, j, k);
        SK6812_Delay_ns(time);
    }
}
