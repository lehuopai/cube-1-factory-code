#ifndef _DHT12_DRIVER_H
#define _DHT12_DRIVER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "i2c_master.h"

#define DHT12_I2C_SCL_IO        	  22
#define DHT12_I2C_SDA_IO            21
#define DHT12_I2C_MASTER_NUM        I2C_NUM_0

#define DHT12_ADDR                  0x5C
#define DHT12_BEGIN_MEASURE         0X00

/**
  * @brief  DHT12采集一次数据
  * @param  Temprature:指针返回温度数据
  * @param  Humi:指针返回湿度数据
  * @retval 校验成功返回0，失败返回1
  */
uint8_t DHT12_Get_Val(float *Temprature, float *Humi);


#endif /* _DHT12_DRIVER_H */