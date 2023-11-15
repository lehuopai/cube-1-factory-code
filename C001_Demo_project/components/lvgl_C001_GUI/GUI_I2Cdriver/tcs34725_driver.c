#include "tcs34725_driver.h"
#include "i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>


/**
  * @brief  TCS34725写数据
  * @param  reg_addr:TCS34725寄存器地址;  这里每个寄存器地址都需要与上COMMAND位
  * @param  len:写入数据的长度; 
  * @param  data_buf:待写入数据的首地址 
  * @retval 参考esp_err_t
  */
static uint8_t TCS34725_I2c_Write(uint8_t reg_addr, uint8_t len, uint8_t *data_buf)
{
    int ret = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (TCS34725_ADDR << 1)|WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr|TCS34725_COMMAND_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_buf, len, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(TCS34725_I2C_MASTER_NUM, cmd, 100/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    // I2c_Check_Err(ret);
    return ret;
}

/**
  * @brief  TCS34725读数据
  * @param  reg_addr:TCS34725寄存器地址;   这里每个寄存器地址都需要与上COMMAND位
  * @param  len:读出数据的长度; 
  * @param  data_buf:待读数据的首地址 
  * @retval 参考esp_err_t
  */
static uint8_t TCS34725_I2c_Read(uint8_t reg_addr, uint8_t len, uint8_t *data_buf)
{
    int ret = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (TCS34725_ADDR << 1)|WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr|TCS34725_COMMAND_BIT, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (TCS34725_ADDR << 1)|READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, data_buf, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(TCS34725_I2C_MASTER_NUM, cmd, 100/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    // I2c_Check_Err(ret);
    return ret;
}

/**
  * @brief  TCS34725检查芯片id
  * @param  void
  * @retval 返回芯片寄存器id中的值
  */
static uint8_t TCS34725_Check_ID(void)
{
    uint8_t id[1] = {0};

	  TCS34725_I2c_Read(TCS34725_ID, 1, id);
    return id[0];
}

/**
  * @brief  TCS34725设置积分时间
  * @param  time : 时间
  * @retval void
  */
void TCS34725_SetIntegrationTime(uint8_t time)
{
    uint8_t cmd = time;
    
    TCS34725_I2c_Write(TCS34725_ATIME, 1, &cmd);
}

/**
  * @brief  TCS34725设置增益
  * @param  time : 增益
  * @retval void
  */
void TCS34725_SetGain(uint8_t gain)
{
    uint8_t cmd = gain;

    TCS34725_I2c_Write(TCS34725_CONTROL, 1, &cmd);
}

/**
  * @brief  TCS34725使能
  * @param  void
  * @retval void
  */
static void TCS34725_Enable(void)
{
    uint8_t cmd = TCS34725_ENABLE_PON;
    
    TCS34725_I2c_Write(TCS34725_ENABLE, 1, &cmd);
    cmd = TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN;
    TCS34725_I2c_Write(TCS34725_ENABLE, 1, &cmd);
    vTaskDelay(3 / portTICK_PERIOD_MS);
}

/**
  * @brief  TCS34725失能
  * @param  void
  * @retval void
  */
static void TCS34725_Disable(void)
{
    uint8_t cmd = 0;
    
    TCS34725_I2c_Read(TCS34725_ENABLE, 1, &cmd);
    cmd = cmd & ~(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
    TCS34725_I2c_Write(TCS34725_ENABLE, 1, &cmd);
}

/**
  * @brief  TCS34725获取单个通道数据
  * @param  void
  * @retval 该通道的转换值
  */
static uint16_t TCS34725_GetChannelData(uint8_t reg)
{
    uint8_t tmp[2] = {0,0};
    uint16_t data = 0;
    
    TCS34725_I2c_Read(reg, 2, tmp);
    data = ((uint16_t)tmp[1] << 8) | tmp[0];
    
    return data;
}

/**
  * @brief  TCS34725获取各个通道数据
  * @param  四个通道的数据
  * @retval 1：成功、0、失败
  */
uint8_t TCS34725_GetRawData(uint16_t *red, uint16_t *green, uint16_t *blue, uint16_t *clear)
{
    uint8_t status[1] = {0};

    status[0] = TCS34725_STATUS_AVALID;

    TCS34725_Enable();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    
    TCS34725_I2c_Read(TCS34725_STATUS, 1, status);
    
    if(status[0] & TCS34725_STATUS_AVALID)
    {
        *clear = TCS34725_GetChannelData(TCS34725_CDATAL);	
        *red = TCS34725_GetChannelData(TCS34725_RDATAL);	
        *green = TCS34725_GetChannelData(TCS34725_GDATAL);	
        *blue = TCS34725_GetChannelData(TCS34725_BDATAL);
        TCS34725_Disable();
        return 1;
    }
    TCS34725_Disable();
    return 0;
}

