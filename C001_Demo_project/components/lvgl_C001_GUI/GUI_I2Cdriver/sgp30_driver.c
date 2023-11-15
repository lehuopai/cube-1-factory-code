#include "sgp30_driver.h"
#include "i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

/**
  * @brief  SGP30发送一个16bit命令
  * @param  cmd_val : 要发送的16bit命令
  * @retval 参考esp_err_t
  */
static uint8_t SGP30_I2c_Write_Cmd(uint16_t cmd_val)
{
    int ret = 0;
    uint8_t cmd_buffer[2];
    cmd_buffer[0] = cmd_val >> 8;
    cmd_buffer[1] = cmd_val;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SGP30_ADDR << 1)|WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, cmd_buffer, 2, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(SGP30_I2C_MASTER_NUM, cmd, 100/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    // I2c_Check_Err(ret);
    return ret;
}

/**
  * @brief  SGP30寄存器读数据
  * @param  len:读出数据的长度; 
  * @param  data_buf:待读数据的首地址 
  * @retval 参考esp_err_t
  */
static uint8_t SGP30_I2c_Read(uint8_t len, uint8_t *data_buf)
{
    int ret = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SGP30_ADDR << 1)|READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, data_buf, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(SGP30_I2C_MASTER_NUM, cmd, 100/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
  * @brief  SGP30发送软复位
  * @param  void
  * @retval 参考esp_err_t
  */
static uint8_t SGP30_Soft_Reset(void)
{   
    int ret = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SGP30_ADDR << 1)|WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, SGP30_SOFT_RESET_CMD, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(SGP30_I2C_MASTER_NUM, cmd, 100/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    // I2c_Check_Err(ret);
    return ret;
}

/**
  * @brief  SGP30初始化
  * @param  void
  * @retval 成功：0， 失败：1
  */
uint8_t SGP30_Init(void)
{  
    if(ESP_OK != SGP30_Soft_Reset())
        return 1;
    
    vTaskDelay(100 / portTICK_PERIOD_MS);
    if(ESP_OK != SGP30_I2c_Write_Cmd(SGP30_INIT_AIR_QUALITY))
        return 1;
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    return 0;
}

/**
  * @brief  SGP30crc校验
  * @param  ----
  * @retval 校验结果
  */
static uint8_t CheckCrc8(uint8_t* const message, uint8_t initial_value)
{
    uint8_t  remainder;	    //余数
    uint8_t  i = 0, j = 0;  //循环变量

    /* 初始化 */
    remainder = initial_value;

    for(j = 0; j < 2;j++){
        remainder ^= message[j];
        /* 从最高位开始依次计算  */
        for (i = 0; i < 8; i++){
            if (remainder & 0x80){
                remainder = (remainder << 1)^SGP30_CRC8_POLYNOMIAL;
            }else{
                remainder = (remainder << 1);
            }
        }
    }
    /* 返回计算的CRC码 */
    return remainder;
}

/**
  * @brief  SGP30采集一次数据
  * @param  co2_val：二氧化碳值
  * @param  tvoc_val：总挥发性有机物值(甲醛)
  * @retval 成功：0， 失败：1
  */
uint8_t SGP30_Get_Value(uint16_t *co2_val, uint16_t *tvoc_val)
{
    uint8_t recv_buf[6]={0,0,0,0,0,0};

    if(ESP_OK != SGP30_I2c_Write_Cmd(SGP30_MEASURE_AIR_QUALITY))
        return 1;
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    if(ESP_OK != SGP30_I2c_Read(6, recv_buf))
        return 1;

     /* 校验接收的测量数据 */
    if (CheckCrc8(&recv_buf[0], 0xFF) != recv_buf[2])
        return 1;

    if (CheckCrc8(&recv_buf[3], 0xFF) != recv_buf[5])
        return 1;

    /* 转换测量数据 */
    *co2_val  = recv_buf[0] << 8 | recv_buf[1];
    *tvoc_val = recv_buf[3] << 8 | recv_buf[4];

    return 0;
}