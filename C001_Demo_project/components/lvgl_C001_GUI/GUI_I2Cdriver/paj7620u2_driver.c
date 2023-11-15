#include "paj7620u2_driver.h"
#include "i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

//上电初始化数组
const uint8_t init_Array[][2] = {
    {0xEF,0x00},{0x37,0x07},{0x38,0x17},{0x39,0x06},{0x41,0x00},{0x42,0x00},{0x46,0x2D},{0x47,0x0F},
    {0x48,0x3C},{0x49,0x00},{0x4A,0x1E},{0x4C,0x20},{0x51,0x10},{0x5E,0x10},{0x60,0x27},{0x80,0x42},
    {0x81,0x44},{0x82,0x04},{0x8B,0x01},{0x90,0x06},{0x95,0x0A},{0x96,0x0C},{0x97,0x05},{0x9A,0x14},
    {0x9C,0x3F},{0xA5,0x19},{0xCC,0x19},{0xCD,0x0B},{0xCE,0x13},{0xCF,0x64},{0xD0,0x21},{0xEF,0x01},
    {0x02,0x0F},{0x03,0x10},{0x04,0x02},{0x25,0x01},{0x27,0x39},{0x28,0x7F},{0x29,0x08},{0x3E,0xFF},
    {0x5E,0x3D},{0x65,0x96},{0x67,0x97},{0x69,0xCD},{0x6A,0x01},{0x6D,0x2C},{0x6E,0x01},{0x72,0x01},
    {0x73,0x35},{0x74,0x00},{0x77,0x01},
};

//手势识别初始化数组
const uint8_t gesture_arry[][2]={
    {0xEF,0x00},{0x41,0x00},{0x42,0x00},{0xEF,0x00},{0x48,0x3C},{0x49,0x00},{0x51,0x10},{0x83,0x20},
    {0x9F,0xF9},{0xEF,0x01},{0x01,0x1E},{0x02,0x0F},{0x03,0x10},{0x04,0x02},{0x41,0x40},{0x43,0x30},
    {0x65,0x96},{0x66,0x00},{0x67,0x97},{0x68,0x01},{0x69,0xCD},{0x6A,0x01},{0x6B,0xB0},{0x6C,0x04},
    {0x6D,0x2C},{0x6E,0x01},{0x74,0x00},{0xEF,0x00},{0x41,0xFF},{0x42,0x01},
};

//接近检测初始化数组
const uint8_t proximity_arry[][2]={
    {0xEF,0x00},{0x41,0x00},{0x42,0x00},{0x48,0x3C},{0x49,0x00},{0x51,0x13},{0x83,0x20},{0x84,0x20},
    {0x85,0x00},{0x86,0x10},{0x87,0x00},{0x88,0x05},{0x89,0x18},{0x8A,0x10},{0x9f,0xf8},{0x69,0x96},
    {0x6A,0x02},{0xEF,0x01},{0x01,0x1E},{0x02,0x0F},{0x03,0x10},{0x04,0x02},{0x41,0x50},{0x43,0x34},
    {0x65,0xCE},{0x66,0x0B},{0x67,0xCE},{0x68,0x0B},{0x69,0xE9},{0x6A,0x05},{0x6B,0x50},{0x6C,0xC3},
    {0x6D,0x50},{0x6E,0xC3},{0x74,0x05},
};

#define INIT_SIZE                       sizeof(init_Array)/2
#define GESTURE_SIZE                    sizeof(gesture_arry)/2
#define PROXIM_SIZE                     sizeof(proximity_arry)/2

/**
  * @brief  PAJ7620U2向寄存器写一个数据
  * @param  reg_addr:PAJ7620U2寄存器地址;  这里每个寄存器地址都需要与上COMMAND位
  * @param  data:待写入的数据
  * @retval 参考esp_err_t
  */
static uint8_t PAJ7620U2_I2c_Write_One(uint8_t reg_addr, uint8_t data)
{
    int ret = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PAJ7620U2_ADDR << 1)|WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(PAJ7620U2_I2C_MASTER_NUM, cmd, 100/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    // I2c_Check_Err(ret);
    return ret;
}

/**
  * @brief  PAJ7620U2读数据
  * @param  reg_addr:PAJ7620U2寄存器地址;   这里每个寄存器地址都需要与上COMMAND位
  * @param  len:读出数据的长度; 
  * @param  data_buf:待读数据的首地址 
  * @retval 参考esp_err_t
  */
static uint8_t PAJ7620U2_I2c_Read(uint8_t reg_addr, uint8_t len, uint8_t *data_buf)
{
    int ret = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PAJ7620U2_ADDR << 1)|WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PAJ7620U2_ADDR << 1)|READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, data_buf, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(PAJ7620U2_I2C_MASTER_NUM, cmd, 100/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    // I2c_Check_Err(ret);
    return ret;
}

/**
  * @brief  PAJ7620U2唤醒
  * @param  void
  * @retval 参考esp_err_t
  */
static uint8_t PAJ7620U2_I2c_WakeUp(void)
{
	  int ret = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PAJ7620U2_ADDR << 1)|WRITE_BIT, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(PAJ7620U2_I2C_MASTER_NUM, cmd, 100/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    // I2c_Check_Err(ret);
    return ret;
}

/**
  * @brief  选择PAJ7620U2 BANK区域
  * @param  bank : 区域名称
  * @retval void
  */
static void PAJ7620U2_Select_Bank(bank_e bank)
{
    if(bank == 0){
        PAJ7620U2_I2c_Write_One(PAJ_REGITER_BANK_SEL, PAJ_BANK0);    //BANK0寄存器区域
    }else{
        PAJ7620U2_I2c_Write_One(PAJ_REGITER_BANK_SEL, PAJ_BANK1);    //BANK1寄存器区域
    } 
}

/**
  * @brief  PAJ7620U2唤醒
  * @param  void
  * @retval 成功：1 失败：0
  */
static uint8_t PAJ7620U2_Wakeup(void)
{ 
    uint8_t data[1] = {0};
    PAJ7620U2_I2c_WakeUp();               //唤醒PAJ7620U2
    vTaskDelay(5 / portTICK_PERIOD_MS);
    PAJ7620U2_I2c_WakeUp();               //唤醒PAJ7620U2
    vTaskDelay(5 / portTICK_PERIOD_MS);
    PAJ7620U2_Select_Bank(BANK0);         //进入BANK0寄存器区域

    PAJ7620U2_I2c_Read(0x00, 1, data);  //读取状态
    if(data[0] != 0x20)                        //唤醒失败
        return 0;              
    return 1;
}

/**
  * @brief  PAJ7620U2初始化
  * @param  void
  * @retval 成功：1 失败：0
  */
uint8_t PAJ7620U2_init(void)
{
    uint8_t i=0;
    uint8_t status=0;
    
    status = PAJ7620U2_Wakeup();          //唤醒PAJ7620U2
    if(!status)
        return 0;

    PAJ7620U2_Select_Bank(BANK0);         //进入BANK0寄存器区域
    for(i=0; i<INIT_SIZE; i++){
        PAJ7620U2_I2c_Write_One(init_Array[i][0], init_Array[i][1]);    //初始化PAJ7620U2
    }
    PAJ7620U2_Select_Bank(BANK0);         //切换回BANK0寄存器区域
    
    return 1;
}

/**
  * @brief  PAJ7620U2手势测试初始化
  * @param  void
  * @retval void
  */
void PAJ7620U2_Gesture_Init(void)
{   
    uint8_t i=0;
    PAJ7620U2_Select_Bank(BANK0);         //进入BANK0寄存器区域
    for(i=0; i<GESTURE_SIZE; i++){
		    PAJ7620U2_I2c_Write_One(gesture_arry[i][0], gesture_arry[i][1]);  //手势识别模式初始化
	  }
    PAJ7620U2_Select_Bank(BANK0);         //切换回BANK0寄存器区域
}

/**
  * @brief  PAJ7620U2获得手势状态
  * @param  gesture_status ： 指针返回手势状态
  * @retval void
  */
void PAJ7620U2_Get_Gesture(uint16_t *gesture_status)
{
    uint8_t gesture_data[2] = {0,0};

    PAJ7620U2_I2c_Read(PAJ_GET_INT_FLAG1, 2, gesture_data);
    *gesture_status = (uint16_t)gesture_data[1]<<8 | gesture_data[0];
}

/**
  * @brief  PAJ7620U2接近测试初始化
  * @param  void
  * @retval void
  */
void PAJ7620U2_Approach_Init(void)
{
    uint8_t i=0;
    PAJ7620U2_Select_Bank(BANK0);         //进入BANK0寄存器区域
    for(i=0; i<PROXIM_SIZE; i++){
		    PAJ7620U2_I2c_Write_One(proximity_arry[i][0], proximity_arry[i][1]);  //手势识别模式初始化
	  }
    PAJ7620U2_Select_Bank(BANK0);         //切换回BANK0寄存器区域
}

/**
  * @brief  PAJ7620U2获得接近物体数据
  * @param  obj_brightness ： 指针返回物体亮度0-255
  * @param  obj_size : 指针返回物体大小0-900
  * @retval void
  */
void PAJ7620U2_Get_Approach(uint8_t *obj_brightness, uint16_t *obj_size)
{
    uint8_t approach_data[3] = {0,0,0};
    PAJ7620U2_I2c_Read(PAJ_GET_OBJECT_BRIGHTNESS, 1, &approach_data[0]);     //读取物体亮度
    PAJ7620U2_I2c_Read(PAJ_GET_OBJECT_SIZE_1, 1, &approach_data[1]);          //读物体大小
    PAJ7620U2_I2c_Read(PAJ_GET_OBJECT_SIZE_2, 1, &approach_data[2]);
  
    *obj_brightness = approach_data[0];
    *obj_size = ((uint16_t)approach_data[2] & 0x0f)<<8 | approach_data[1];
}