#ifndef __IP5306_DRIVER_H
#define __IP5306_DRIVER_H

#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define IP5306_I2C_SCL_IO            4
#define IP5306_I2C_SDA_IO            13
#define IP5306_I2C_MASTER_NUM        I2C_NUM_1  

#define IP5306_ADDR                   0x75       //IP5306IIC地址
#define IP5306_CHECK_POWER 			      0x78       //IP5306读电量寄存器
#define IP5306_CHECK_CHANGE 		      0x70       //IP5306读是否充电寄存器
#define IP5306_CHECK_FULL             0x71       //IP5306读是否充满寄存器

 /**
  * @brief  Ip5306读电量
  * @param  void
  * @retval 返回五档电量值
  */
uint8_t Ip5306_Check_Power(void);

/**
  * @brief  Ip5306判断是否在充电
  * @param  void
  * @retval 1：充电中 0：未充电
  */
uint8_t Ip5306_Check_Charge(void);

/**
  * @brief  Ip5306判断是否充满。
  * @param  void
  * @retval 1：已充满 0：未充满
  */
uint8_t Ip5306_Check_Full(void);


#endif /* __IP5306_DRIVER_H */