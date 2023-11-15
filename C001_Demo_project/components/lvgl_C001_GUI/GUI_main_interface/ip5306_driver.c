#include "ip5306_driver.h"
#include "i2c_master.h"
#include <stdio.h>

/**
  * @brief  Ip5306读数据
  * @param  reg_addr:Ip5306寄存器地址; 
  * @param  len:读出数据的长度; 
  * @param  data_buf:待读数据的首地址 
  * @retval 参考esp_err_t
  */
static uint8_t Ip5306_I2c_Read(uint8_t reg_addr, uint8_t len, uint8_t *data_buf)
{
    int ret = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (IP5306_ADDR << 1)|WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (IP5306_ADDR << 1)|READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, data_buf, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(IP5306_I2C_MASTER_NUM, cmd, 100/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    // I2c_Check_Err(ret);
    return ret;
}

/**
  * @brief  Ip5306读电量
  * @param  void
  * @retval 返回五档电量值
  */
uint8_t Ip5306_Check_Power(void)
{
    uint8_t data = 0;
    Ip5306_I2c_Read(IP5306_CHECK_POWER, 1, &data);
    if(data == 0x00)
        return 100;
    else if(data == 0x80)
        return 75;
    else if(data == 0xC0)
        return 50;
    else if(data == 0xE0)
        return 25;
    else
        return 0;
}

/**
  * @brief  Ip5306判断是否在充电
  * @param  void
  * @retval 1：充电中 0：未充电
  */
uint8_t Ip5306_Check_Charge(void)
{
    uint8_t data = 0;
    Ip5306_I2c_Read(IP5306_CHECK_CHANGE, 1, &data);
    if(data>>3 & 0x01){
      return 1;
    }
    return 0;
}

/**
  * @brief  Ip5306判断是否充满。
  * @param  void
  * @retval 1：已充满 0：未充满
  */
uint8_t Ip5306_Check_Full(void)
{
    uint8_t data = 0;
    Ip5306_I2c_Read(IP5306_CHECK_FULL, 1, &data);
    if(data>>3 & 0x01){
      return 1;
    }
    return 0;
}