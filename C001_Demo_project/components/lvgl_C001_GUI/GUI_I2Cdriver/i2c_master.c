#include "i2c_master.h"
#include <stdio.h>

/**
  * @brief  初始化I2C主机
  * @param  I2c_num :i2c编号
  * @param  sda_num :sda引脚编号
  * @param  scl_num :scl引脚编号
  * @retval void
  */
void I2c_Master_Init(uint8_t I2c_num, uint8_t sda_num, uint8_t scl_num)
{
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_num,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = scl_num,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2c_num, &i2c_conf);
    i2c_driver_install(I2c_num, I2C_MODE_MASTER, 0, 0, 0);
}

/**
  * @brief  检测设备是否存在
  * @param  I2c_addr:i2c设备地址
  * @param  test_val:用来测试的值，应防止与器件寄存器地址冲突
  * @retval 存在返回0，不存在返回1
  */
uint8_t I2C_ALIVE(uint8_t I2c_addr, uint8_t test_val)
{
  int ret = 0;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (I2c_addr << 1)|WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, test_val, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100/portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  // I2c_Check_Err(ret);
  if(ret == ESP_OK) {
    return 0;
  }
  return 1;
}

/**
  * @brief  打印i2c cmd执行结果
  * @param  val:错误号
  * @retval void
  */
void I2c_Check_Err(esp_err_t val)
{
  switch(val){
    case ESP_OK:
      printf("ESP_OK 成功\n");
      break;
    case ESP_ERR_INVALID_ARG:
      printf("ESP_ERR_INVALID_ARG 参数错误\n");
      break;
    case ESP_FAIL:
      printf("ESP_FAIL 发送命令错误，从机未确认传输。\n");
      break;
    case ESP_ERR_INVALID_STATE:
      printf("ESP_ERR_INVALID_STATE I2C 驱动程序未安装或不在主模式下。\n");
      break;
    case ESP_ERR_TIMEOUT:
      printf("ESP_ERR_TIMEOUT 由于总线繁忙，操作超时。\n");
      break;
    default:
      break;
  }
}

