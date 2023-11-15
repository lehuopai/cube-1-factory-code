#ifndef __I2C_MASTER_H
#define __I2C_MASTER_H

#include "driver/i2c.h"
#include "esp_err.h"

#define I2C_SCL_IO            22
#define I2C_SDA_IO            21
#define I2C_MASTER_NUM        I2C_NUM_0

#define WRITE_BIT                   I2C_MASTER_WRITE
#define READ_BIT                    I2C_MASTER_READ 
#define ACK_CHECK_EN                0x01
#define ACK_CHECK_DIS               0x00
#define ACK_VAL                     0x00
#define NACK_VAL                    0x01

/**
  * @brief  初始化I2C主机
  * @param  I2c_num :i2c编号
  * @param  sda_num :sda引脚编号
  * @param  scl_num :scl引脚编号
  * @retval void
  */
void I2c_Master_Init(uint8_t I2c_num, uint8_t sda_num, uint8_t scl_num);

/**
  * @brief  检测设备是否存在
  * @param  I2c_addr:i2c设备地址
  * @param  test_val:用来测试的值，应防止与器件寄存器地址冲突
  * @retval 存在返回0，不存在返回1
  */
uint8_t I2C_ALIVE(uint8_t I2c_addr, uint8_t test_val);

/**
  * @brief  打印i2c cmd执行结果
  * @param  val:错误号
  * @retval void
  */
void I2c_Check_Err(esp_err_t val);

#endif /* __I2C_MASTER_H */