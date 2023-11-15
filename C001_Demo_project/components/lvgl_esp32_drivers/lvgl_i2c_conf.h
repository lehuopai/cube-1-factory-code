/**
 * @file lvgl_i2c_config.h
 */

#ifndef LVGL_I2C_CONF_H
#define LVGL_I2C_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "../lvgl/lvgl/src/lv_hal/lv_hal.h"

/*********************
 *      DEFINES
 *********************/
#define TOUCH_I2C_PORT           I2C_NUM_0
#define TOUCH_I2C_SDA            21
#define TOUCH_I2C_SCL            22
#define TOUCH_I2C_SPEED_HZ       100000

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 * GLOBAL PROTOTYPES
 **********************/


/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LVGL_I2C_CONF_H*/
