#ifndef __SGP30_DRIVER_H
#define __SGP30_DRIVER_H

#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SGP30_I2C_SCL_IO            22
#define SGP30_I2C_SDA_IO            21
#define SGP30_I2C_MASTER_NUM        I2C_NUM_0

#define SGP30_ADDR                  0x58

#define SGP30_SOFT_RESET_CMD        0x06          //软复位命令
#define SGP30_INIT_AIR_QUALITY      0x2003        //初始化空气质量测量命令
#define SGP30_MEASURE_AIR_QUALITY   0x2008        //开始空气质量测量命令
#define SGP30_CRC8_POLYNOMIAL       0x31

/**
  * @brief  SGP30初始化
  * @param  void
  * @retval 成功：0， 失败：1
  */
uint8_t SGP30_Init(void);

/**
  * @brief  SGP30采集一次数据
  * @param  co2_val：二氧化碳值
  * @param  tvoc_val：总挥发性有机物值(甲醛)
  * @retval 成功：0， 失败：1
  */
uint8_t SGP30_Get_Value(uint16_t *co2_val, uint16_t *tvoc_val);

#endif /* __SGP30_DRIVER_H */