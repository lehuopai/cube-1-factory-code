#include "bh1750fvi_driver.h"
#include <stdio.h>
#include "i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
  * @brief  BH1750FVI写指令
  * @param  cmd_data:BH1750FVI指令名称; 
  * @retval 参考esp_err_t
  */
static uint8_t BH1750FVI_I2c_Write(uint8_t cmd_data)
{
    int ret = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BH1750FVI_ADDR << 1)|WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, cmd_data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(BH1750FVI_I2C_MASTER_NUM, cmd, 100/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    // I2c_Check_Err(ret);
    return ret;
}

/**
  * @brief  BH1750FVI读数据
  * @param  len:读出数据的长度; 
  * @param  data_buf:待读数据的首地址 
  * @retval 参考esp_err_t
  */
static uint8_t BH1750FVI_I2c_Read(uint8_t len, uint8_t *data_buf)
{
    int ret = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BH1750FVI_ADDR << 1)|READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, data_buf, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(BH1750FVI_I2C_MASTER_NUM, cmd, 100/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    // I2c_Check_Err(ret);
    return ret;
}

/**
  * @brief  BH1750FVI读一次亮度数据
  * @param  void
  * @retval 返回读到的亮度数据，（0-65535），单位: lx
  */
float BH1750FVI_Read_Data(void)
{ 
    uint8_t buf[2] = {0};
    uint32_t dis_data;               
    float temp;
    
    BH1750FVI_I2c_Write(BH1750FVI_POWER_ON);
    BH1750FVI_I2c_Write(BH1750FVI_CONTI_H_MODE);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    BH1750FVI_I2c_Read(2, buf);

    dis_data = buf[0];
    dis_data = (dis_data<<8)+buf[1];              //合成数据
    temp = (float)dis_data/1.2;                   //获得整数部分
    temp += (float)((int)(10*dis_data/1.2)%10)/10;       //获得小数部分

    return temp;
}
