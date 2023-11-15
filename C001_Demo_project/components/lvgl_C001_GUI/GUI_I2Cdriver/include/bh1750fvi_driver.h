#ifndef __BH1750FVI_DRIVER_H
#define __BH1750FVI_DRIVER_H

#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BH1750FVI_I2C_SCL_IO            22
#define BH1750FVI_I2C_SDA_IO            21
#define BH1750FVI_I2C_MASTER_NUM        I2C_NUM_0

#define BH1750FVI_ADDR                  0x23		//bh1750fvi iic地址

#define BH1750FVI_POWER_ON 				      0x01		//bh1750fvi 电源开
#define BH1750FVI_POWER_DOWN 			      0x00		//bh1750fvi 电源关
#define BH1750FVI_RESET 				        0x07		//bh1750fvi 复位
#define BH1750FVI_CONTI_H_MODE 			    0x10		//bh1750fvi 11x分辨率开始测量

/**
  * @brief  BH1750FVI读一次亮度数据
  * @param  void
  * @retval 返回读到的亮度数据，（0-65535），单位: lx
  */
float BH1750FVI_Read_Data(void);

#endif /* __BH1750FVI_DRIVER_H */