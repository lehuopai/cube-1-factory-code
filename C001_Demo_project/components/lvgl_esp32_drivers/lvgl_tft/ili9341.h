#ifndef ILI9341_H
#define ILI9341_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// #include "lvgl/lvgl.h"
#include "../../lvgl/lvgl/lvgl.h"
#include "../lvgl_helpers.h"

#define ILI9341_DC      27        //DC控制引脚
#define ILI9341_RST     33        //硬件复位控制引脚
#define ILI9341_BCKL    32        //背光控制引脚

#define ILI9341_ENABLE_BACKLIGHT_CONTROL  1       //是否开启背光控制
#define ILI9341_BCKL_ACTIVE_LVL           1       //在高(1)逻辑电平时是否打开背光      
#define ILI9341_INVERT_COLORS             1       //是否颜色反转
//LV_INVERT_COLORS

#define ILI9341_DISPLAY_ORIENTATION       2       //ILI9341方向选择  
//0.竖屏 1.倒立竖屏 2.横屏 3.倒立横屏
//LV_DISPLAY_ORIENTATION

/**
  * @brief  初始化ili9341
  * @param  void
  * @retval void
  * @note   写入初始化设置ili9341命令
  */
void ili9341_init(void);

/**
  * @brief 	ili9341颜色填充
  * @param  drv：显示驱动结构，由HAL注册
  * @param	area：表示屏幕的一个区域。
  * @param	color_map：颜色数据
  * @retval void
  * @note   移植GUI则需要使用此函数来驱动绘制
  */
void ili9341_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map);

/**
  * @brief  ili9341背光控制
  * @param  backlight：是否开启
  * @retval void
  */
void ili9341_enable_backlight(bool backlight);

/**
  * @brief  ili9341进入休眠
  * @param  void
  * @retval void
  */
void ili9341_sleep_in(void);

/**
  * @brief  ili9341退出休眠
  * @param  void
  * @retval void
  */
void ili9341_sleep_out(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*ILI9341_H*/
